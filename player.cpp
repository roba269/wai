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
    pid_t pid;
    if ((pid = fork()) == 0) {
        // child
        execl(fullname.c_str(), fullname.c_str(), NULL);
    }
    return 0;
}

void PlayerComputer::SendMessage(const char *msg)
{
    // TODO: write msg to pipe
}

void PlayerComputer::RecvMessage(char *&msg)
{
    // TODO: read msg from pipe
}

