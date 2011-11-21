#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "player.h"

const std::string prefix = "/var/ai/";

int PlayerComputer::LoadAI(std::string ai_name)
{
    std::string fullname = prefix + ai_name;
    if (access(fullname.c_str(), R_OK | X_OK))
        return -1;
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
        execl(fullname.c_str(), fullname.c_str(), NULL);
    }
    return 0;
}

void PlayerComputer::SendMessage(const char *msg)
{
    // TODO: write msg to pipe
    write(m_outfd, msg, strlen(msg));
}

void PlayerComputer::RecvMessage(char *&msg, int maxlen)
{
    // TODO: read msg from pipe
    read(m_infd, msg, maxlen);
}

