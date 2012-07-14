#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "common_define.h"
#include "sandbox.h"
#include "syscalls.h"

const int MEMORY_LIMIT = 128 * 1024 * 1024; // in bytes
const int TIME_LIMIT = 10;  // in seconds

static int set_quota() {
    struct rlimit lim;
    // no core dump file
    if (getrlimit(RLIMIT_CORE, &lim) < 0) return -1;
    lim.rlim_cur = 0;
    if (setrlimit(RLIMIT_CORE, &lim) < 0) return -1;
    // set memory limit
    if (getrlimit(RLIMIT_AS, &lim) < 0) return -1;
    lim.rlim_cur = MEMORY_LIMIT;
    if (setrlimit(RLIMIT_AS, &lim) < 0) return -1;
    // set time limit
    /*
    struct itimerval timerval;
    timerval.it_interval.tv_sec = 0;
    timerval.it_interval.tv_usec = 0;
    timerval.it_value.tv_sec = TIME_LIMIT;
    timerval.it_value.tv_usec = 0;
    if (setitimer(ITIMER_PROF, &timerval, NULL) < 0) return -1;
    */
    if (getrlimit(RLIMIT_CPU, &lim) < 0) return -1;
    lim.rlim_cur = TIME_LIMIT;
    if (setrlimit(RLIMIT_CPU, &lim) < 0) return -1;
    return 0;
}

Sandbox::Sandbox(std::string path) : m_idx(0), m_len(0) {
    m_path = path;
}

Sandbox::~Sandbox() {

}

int Sandbox::Run() {
    if (access(m_path.c_str(), R_OK | X_OK)) {
        fprintf(stderr, "Cannot access %s\n", m_path.c_str());
        return -1;
    }
    fprintf(stderr, "%s begin to run\n", m_path.c_str());
    int fd1[2], fd2[2];
    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
        fprintf(stderr, "Error on pipe()\n");
        return -1;
    }
    pid_t child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "error on fork()\n");
        return -1;
    } else if (child_pid == 0) {
        close(fd1[1]);
        close(fd2[0]);
        // close(info_fd[0]);
        pid_t grandson_pid = fork();
        if (grandson_pid < 0) {
            fprintf(stderr, "error on fork()\n");
            return -1;
        } else if (grandson_pid == 0) {
            // grandson: the actual executable
            // close(info_fd[0]);
            if (fd1[0] != STDIN_FILENO) {
                dup2(fd1[0], STDIN_FILENO);
                close(fd1[0]);
            }
            if (fd2[1] != STDOUT_FILENO) {
                dup2(fd2[1], STDOUT_FILENO);
                close(fd2[1]);
            }
            // setrlimit
            if (set_quota() < 0) {
                fprintf(stderr, "set_quota() error\n");
            }
            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            execl(m_path.c_str(), m_path.c_str(), NULL);
            fprintf(stderr, "execl failed, errno: %d\n", errno);
            return -1;  
        } else {
            fprintf(stderr, "%d: i am the sandbox of %d\n", getpid(), grandson_pid);
            // son: the sandbox
            close(fd1[0]);
            close(fd2[1]);
            int st, in_call = 0;
            while (1) {
                struct rusage ru;
                wait4(grandson_pid, &st, 0, &ru);
                // SetUsedTime(ru.ru_utime.tv_sec * 1000 +
                //         ru.ru_utime.tv_usec / 1000);
                if (WIFEXITED(st)) {
                    m_exit_flag = EXIT_NORMAL;
                    fprintf(stderr, "exit_normal\n");
                    break;
                }
                if (WIFSIGNALED(st) || (WIFSTOPPED(st) && WSTOPSIG(st) != SIGTRAP)) {
                    if (WIFSTOPPED(st)) {
                        switch (WSTOPSIG(st)) {
                            case SIGXCPU:
                                fprintf(stderr, "SIGXCPU\n");
                                m_exit_flag = EXIT_TLE;
                                break;
                            case SIGPIPE:
                                // match is over, the match process is terminated
                                m_exit_flag = EXIT_NORMAL;
                                break;
                            default:
                                fprintf(stderr, "exit: WSTOPSIG(st):%d\n", WSTOPSIG(st));
                                m_exit_flag = EXIT_RE;
                        }
                    }
                    else if (WIFSIGNALED(st)) {
                        fprintf(stderr, "termed by sig: %d\n", WTERMSIG(st));
                        m_exit_flag = EXIT_RE;
                    } else {
                        fprintf(stderr, "termed by sig: %d\n", WTERMSIG(st));
                    }
                    ptrace(PTRACE_KILL, grandson_pid, NULL, NULL);
                    break;
                }
                if (in_call == 0) {
#ifdef __x86_64__
                    long long orig_eax = ptrace(PTRACE_PEEKUSER, grandson_pid,
                            8 * ORIG_RAX, NULL);
#else
                    int orig_eax = ptrace(PTRACE_PEEKUSER, grandson_pid, 
                            4 * ORIG_EAX, NULL);
#endif
                    assert(orig_eax >= 0 && orig_eax < 512);
                    /*
                    if (--m_limit[orig_eax] < 0) {
                        m_exit_flag = EXIT_RF;
                        fprintf(stderr, "Sys call %d reach limit\n", 
                                static_cast<int>(orig_eax));
                        break;
                    }
                    */
                    ++m_stat[orig_eax];
                    // fprintf(stderr, "m_stat[%d]:%d\n", orig_eax,
                    //          m_stat[orig_eax]);
                    in_call = 1;
                } else {
                    in_call = 0;
                }
                if (m_exit_flag == EXIT_RF) {
                    ptrace(PTRACE_KILL, grandson_pid, NULL, NULL);
                    break;
                }
                ptrace(PTRACE_SYSCALL, grandson_pid, NULL, NULL);
            }
            // char buf[16];
            // sprintf(buf, "%d %d\n", GetID(), GetUsedTime());
            // write(GetInfoFd(), buf, strlen(buf));
            fprintf(stderr, "%d: i am exited\n", getpid());
            exit(0);    // the sandbox process exit
        }
    } else {
        // parent
        close(fd1[0]);
        close(fd2[1]);
        recv_fd = fd2[0];
        send_fd = fd1[1];
    }
    return 0;
}

int Sandbox::Send(char *buf) {
    // fprintf(stderr, "%d: write to fd:%d {%s}\n", getpid(), send_fd, buf);
    strcat(buf, "\n");
    write(send_fd, buf, strlen(buf));
    return 0;
}

int Sandbox::Recv(char *buf, int max_len) {
    int i;
    for (i = 0 ; i < max_len ; ++i) {
        if (_RecvChar(buf) == 0 || *buf == '\n') {
            *buf = 0;
            return i;
        }
        ++buf;
    }
    *buf = 0;
    return i;
}

ExitFlagType Sandbox::GetExitType() {
    return m_exit_flag;
}

int Sandbox::_RecvChar(char *buf) {
    if (m_idx == m_len) {
        m_len = read(recv_fd, m_buf, sizeof(m_buf));
        m_idx = 0;
        if (m_len < 0) {
            assert(false);
        }
        if (m_len == 0) return 0;
    }
    *buf = m_buf[m_idx++];
    return 1;
}

void Sandbox::_InitSyscallSpec() {
    memset(m_stat, 0, sizeof(m_stat));
    memset(m_limit, 0, sizeof(m_limit));
    for (int i = 0 ; g_spec[i].number != -1 ; ++i) {
        m_limit[g_spec[i].number] = g_spec[i].limit;
    }
}

