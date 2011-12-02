#include <string>
#include <cstdio>
#include <cstdlib>
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
#include <cstring>
#include <cassert>
#include "player.h"
#include "syscalls.h"
#include "common_define.h"

// FIXME: fprintf should be replaced by write

// const std::string prefix = "/var/ai/";
const std::string prefix = "./";
const int MEMORY_LIMIT = 128 * 1024 * 1024; // in bytes
const int TIME_LIMIT = 10;  // in seconds

Player::Player() : m_id(0), m_infd(0), m_outfd(0), m_info_fd(0) {
    
}

PlayerComputer::PlayerComputer() : m_exit_flag(EXIT_NONE) { }

void PlayerComputer::InitSyscallSpec()
{
    memset(m_stat, 0, sizeof(m_stat));
    memset(m_limit, 0, sizeof(m_limit));
    for (int i = 0 ; g_spec[i].number != -1 ; ++i) {
        m_limit[g_spec[i].number] = g_spec[i].limit;
    }
}

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
    lim.rlim_cur = 1;
    if (setrlimit(RLIMIT_CPU, &lim) < 0) return -1;
    return 0;
}

int PlayerComputer::LoadAI(std::string ai_name, int id)
{
    SetID(id);
    InitSyscallSpec();   
    std::string fullname = prefix + ai_name;
    if (access(fullname.c_str(), R_OK | X_OK)) {
        printf("Cannot access %s\n", fullname.c_str());
        return -1;
    }
    printf("Fullname: %s\n", fullname.c_str());
    // TODO: hanlde SIGPIPE
    int fd1[2], fd2[2], info_fd[2];
    if (pipe(fd1) < 0 || pipe(fd2) < 0 || pipe(info_fd) < 0) {
        fprintf(stderr, "Error on pipe()\n");
        return -1;
    }
    pid_t child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "Error on fork()\n");
        return -1;
    } else if (child_pid == 0) {
        close(fd1[1]);
        close(fd2[0]);
        close(info_fd[0]);
        pid_t grand_pid = fork();
        if (grand_pid < 0) {
            fprintf(stderr, "Error on fork()\n");
            return -1;
        } else if (grand_pid == 0) {
            // generation 3: the actual executable
            close(info_fd[0]);
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
            execl(fullname.c_str(), fullname.c_str(), NULL);
            fprintf(stderr, "execl failed, errno: %d\n", errno);
            return -1;  
        } else {
            // generation 2: the sandbox
            close(fd1[0]);
            close(fd2[1]);
            int st, in_call = 0;
            while (1) {
                wait(&st);
                if (WIFEXITED(st)) {
                    m_exit_flag = EXIT_NORMAL;
                    break;
                }
                if (WIFSIGNALED(st) || (WIFSTOPPED(st) && WSTOPSIG(st) != 5)) {
                    if (WIFSTOPPED(st)) {
                        switch (WSTOPSIG(st)) {
                            case SIGXCPU:
                                m_exit_flag = EXIT_TLE;
                                break;
                            default:
                                m_exit_flag = EXIT_RE;
                        }
                    }
                    else if (WIFSIGNALED(st))
                            fprintf(stderr, "termed by signal: %d\n", WTERMSIG(st));
                    ptrace(PTRACE_KILL, grand_pid, NULL, NULL);
                    break;
                }
                if (in_call == 0) {
#ifdef __x86_64__
                    long long orig_eax = ptrace(PTRACE_PEEKUSER, grand_pid,
                            8 * ORIG_RAX, NULL);
#else
                    int orig_eax = ptrace(PTRACE_PEEKUSER, grand_pid, 
                            4 * ORIG_EAX, NULL);
#endif
                    assert(orig_eax >= 0 && orig_eax < 512);
                    if (--m_limit[orig_eax] < 0) {
                        m_exit_flag = EXIT_RF;
                        fprintf(stderr, "Sys call %d reach limit\n", 
                                static_cast<int>(orig_eax));
                        break;
                    }
                    ++m_stat[orig_eax];
                    // fprintf(stderr, "m_stat[%d]:%d\n", orig_eax,
                    //          m_stat[orig_eax]);
                    in_call = 1;
                } else {
                    in_call = 0;
                }
                if (m_exit_flag == EXIT_RF) {
                    ptrace(PTRACE_KILL, grand_pid, NULL, NULL);
                    break;
                }
                ptrace(PTRACE_SYSCALL, grand_pid, NULL, NULL);
            }
            char buf[16];
            sprintf(buf, "%d\n", GetID());
            write(GetInfoFd(), buf, strlen(buf));
            exit(0);    // the sandbox process exit
        }
    } else {
        // generation 1: main
        close(fd1[0]);
        close(fd2[1]);
        close(info_fd[1]);
        SetInputFd(fd2[0]);
        SetOutputFd(fd1[1]);
        SetInfoFd(info_fd[0]);
        m_sandbox_pid = child_pid;
    }

    return child_pid;
}
/*
void PlayerComputer::SendMessage(const char *msg)
{
    // TODO: write msg to pipe
    // printf("Send %s", msg);
    write(m_outfd, msg, strlen(msg));
}

void PlayerComputer::RecvMessage(char *msg, int maxlen)
{
    // TODO: read msg from pipe
    int n = read(m_infd, msg, maxlen);
    msg[n] = 0;
    // printf("Recv %s", msg);
}
*/
void PlayerComputer::Kill() 
{
    kill(m_sandbox_pid, SIGKILL);
}

