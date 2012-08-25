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
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "common_define.h"
#include "sandbox.h"
#include "syscalls.h"

const int MEMORY_LIMIT = 32 * 1024 * 1024; // in bytes
const int TIME_LIMIT = 60;  // in seconds

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
    _InitSyscallSpec();
}

Sandbox::~Sandbox() {

}

int Sandbox::Run(bool is_rf) {
    if (access(m_path.c_str(), R_OK | X_OK)) {
        fprintf(stderr, "Cannot access %s\n", m_path.c_str());
        return -1;
    }
    fprintf(stderr, "%s begin to run\n", m_path.c_str());
    int fd1[2], fd2[2], info_fd[2];
    if (pipe(fd1) < 0 || pipe(fd2) < 0 || pipe(info_fd) < 0) {
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
        close(info_fd[0]);
        pid_t grandson_pid = fork();
        if (grandson_pid < 0) {
            fprintf(stderr, "error on fork()\n");
            return -1;
        } else if (grandson_pid == 0) {
            // grandson: the actual executable
            close(info_fd[1]);
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
                                m_exit_flag = EXIT_TLE;
                                fprintf(stderr, "pid:%d m_exit_flag:%d\n", getpid(), EXIT_TLE);
                                break;
                            case SIGPIPE:
                                // match is over, the match process is terminated
                                m_exit_flag = EXIT_NORMAL;
                                break;
                            default:
                                fprintf(stderr, "pid:%d exit: WSTOPSIG(st):%d\n", getpid(), WSTOPSIG(st));
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
                    if (is_rf && --m_limit[orig_eax] < 0) {
                        m_exit_flag = EXIT_RF;
                        fprintf(stderr, "pid:%d Sys call %d reach limit\n", 
                                getpid(), static_cast<int>(orig_eax));
                    }
                    ++m_stat[orig_eax];
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
            char buf[16];
            // sprintf(buf, "%d %d\n", GetID(), GetUsedTime());
            // write(GetInfoFd(), buf, strlen(buf));
            fprintf(stderr, "%d: the proc in sandbox exit type: %d\n", getpid(), m_exit_flag);
            fprintf(stderr, "%d: i am exited\n", getpid());
            sprintf(buf, "%d\n", m_exit_flag);
            write(info_fd[1], buf, strlen(buf));
            exit(0);    // the sandbox process exit
        }
    } else {
        // parent
        close(fd1[0]);
        close(fd2[1]);
        close(info_fd[1]);
        recv_fd = fd2[0];
        send_fd = fd1[1];
        recv_info_fd = info_fd[0];
    }
    return 0;
}

int Sandbox::Send(char *buf) {
    strcat(buf, "\n");
    // The pipe may be broken because play proc exited.
    if (write(send_fd, buf, strlen(buf)) == -1) {
        fprintf(stderr, "write failed, errno: %d\n", errno);
    }
    return 0;
}

int Sandbox::Recv(char *buf, int max_len, ExitFlagType &exit_flag) {
    int i;
    for (i = 0 ; i < max_len ; ++i) {
        if (_RecvChar(buf, exit_flag) == 0 || *buf == '\n') {
            if (exit_flag != 0) return 0;
            *buf = 0;
            return i;
        }
        ++buf;
    }
    *buf = 0;
    return i;
}

ExitFlagType Sandbox::_GetExitType() {
    // GetExitType should read from the sandbox process
    char buf[16];
    int tmp;
    read(recv_info_fd, buf, sizeof(buf));
    sscanf(buf, "%d", &tmp);
    m_exit_flag = (ExitFlagType)tmp;
    fprintf(stderr, "%d: GetExitType be called, return: %d\n", getpid(), m_exit_flag);
    return m_exit_flag;
}

int Sandbox::_RecvChar(char *buf, ExitFlagType &exit_flag) {
    if (m_idx == m_len) {
        exit_flag = EXIT_NONE;
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(recv_info_fd, &readset);
        FD_SET(recv_fd, &readset);
        timeval tv;
        // Timeval is set large enough to make sure the elapsed
        // time is long enough comparing with the CPU time limit.
        // If timed out, we can consider that the user process is
        // blocked without consuming CPU time. It may be because
        // of forgetting to flush stdout, for example.
        // Note that this case can't be detected by `ptrace` of
        // sandbox, so we need `select`.
        tv.tv_sec = TIME_LIMIT * 2; tv.tv_usec = 1000;
        int cnt = select(std::max(recv_info_fd, recv_fd) + 1,
            &readset, NULL, NULL, &tv);
        if (cnt == -1) {
            fprintf(stderr, "Select failed.");
            return 0;
        }
        if (cnt == 0) {
            // time out
            exit_flag = EXIT_TLE;
            return 0;
        } else if (FD_ISSET(recv_info_fd, &readset)) {
            exit_flag = _GetExitType();
            return 0;
        } else if (FD_ISSET(recv_fd, &readset)) {
            m_len = read(recv_fd, m_buf, sizeof(m_buf));
            m_idx = 0;
            if (m_len < 0) {
                assert(false);
            }
            if (m_len == 0) return 0;
        }
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

