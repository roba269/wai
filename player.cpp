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
    m_infd(0), m_outfd(0), m_sandbox_pid(0), m_exit_flag(EXIT_NONE) { }

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
    int fd1[2], fd2[2];
    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
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
        pid_t grand_pid = fork();
        if (grand_pid < 0) {
            fprintf(stderr, "Error on fork()\n");
            return -1;
        } else if (grand_pid == 0) {
            // generation 3: the actual executable
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
                if (in_call == 0) {
                    int orig_eax = ptrace(PTRACE_PEEKUSER, grand_pid, 
                            4 * ORIG_EAX, NULL);
                    assert(orig_eax >= 0 && orig_eax < 512);
                    if (--m_limit[orig_eax] < 0) {
                        m_exit_flag = EXIT_RF;
                        fprintf(stderr, "Sys call %d reach limit\n", orig_eax);
                        break;
                    }
                    ++m_stat[orig_eax];
                    // fprintf(stderr, "m_stat[%d]:%d\n", orig_eax,
                    //         m_stat[orig_eax]);
                    in_call = 1;
                } else {
                    in_call = 0;
                }
                ptrace(PTRACE_SYSCALL, grand_pid, NULL, NULL);
            }
        }
    } else {
        // generation 1: main
        close(fd1[0]);
        close(fd2[1]);
        m_infd = fd2[0];
        m_outfd = fd1[1];
        m_sandbox_pid = child_pid;
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
    kill(m_sandbox_pid, SIGKILL);
}

