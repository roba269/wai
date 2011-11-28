#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <cstring>
#include <cassert>
#include "player.h"
#include "syscalls.h"
#include "common_define.h"

// const std::string prefix = "/var/ai/";
const std::string prefix = "./";

PlayerComputer::PlayerComputer() :
    m_infd(0), m_outfd(0), m_child_pid(0), m_exit_flag(EXIT_NONE) { }

void PlayerComputer::InitSyscallSpec()
{
    memset(m_stat, 0, sizeof(m_stat));
    memset(m_limit, 0, sizeof(m_limit));
    for (int i = 0 ; g_spec[i].number != 0 ; ++i) {
        m_limit[g_spec[i].number] = g_spec[i].limit;
    }
}

int PlayerComputer::LoadAI(std::string ai_name)
{
    InitSyscallSpec();   
    std::string fullname = prefix + ai_name;
    if (access(fullname.c_str(), R_OK | X_OK)) {
        printf("Cannot access %s\n", fullname.c_str());
        return -1;
    }
    printf("Fullname: %s\n", fullname.c_str());
    // TODO: hanlde SIGPIPE
    pid_t pid;
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);
    if ((pid = fork()) > 0) {
        // parent
        close(fd1[0]);
        close(fd2[1]);
        m_infd = fd2[0];
        m_outfd = fd1[1];
        m_child_pid = pid;

        int st, in_call = 0;
        while (1) {
            wait(&st);
            if (WIFEXITED(st)) {
                m_exit_flag = EXIT_NORMAL;
                break;
            }
            if (in_call == 0) {
                int orig_eax = ptrace(PTRACE_PEEKUSER, pid, 4*ORIG_EAX, NULL);
                assert(orig_eax >= 0 && orig_eax < 512);
                if (--m_limit[orig_eax] < 0) {
                    m_exit_flag = EXIT_RF;
                    fprintf(stderr, "Sys call %d reach the limit", orig_eax);
                    break;
                }
                ++m_stat[orig_eax];
                in_call = 1;
            } else {
                in_call = 0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        }
    } else if (pid == 0) {
        // child
        close(fd1[1]);
        close(fd2[0]);
        if (fd1[0] != STDIN_FILENO) {
            dup2(fd1[0], STDIN_FILENO);
            close(fd1[0]);
        }
        if (fd2[1] != STDOUT_FILENO) {
            dup2(fd2[1], STDOUT_FILENO);
            close(fd2[1]);
        }

        // setrlimit

        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(fullname.c_str(), fullname.c_str(), NULL);
        printf("execl failed, errno: %d\n", errno);
    }
    return 0;
}

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

void PlayerComputer::Kill() 
{
    printf("Sys calls for pid %d", getpid());
    for (int i = 1 ; i < 512 ; i++) {
        if (m_stat[i]) {printf("No %d: %d\n", i, m_stat[i]);}
    }
    kill(m_child_pid, SIGKILL);
}

