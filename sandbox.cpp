#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#include "sandbox.h"

Sandbox::Sandbox(std::string path) {
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
        execl(m_path.c_str(), m_path.c_str(), NULL);
        assert(false);  // never come here
        return -1;
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
    write(send_fd, buf, strlen(buf));
    return 0;
}

int Sandbox::Recv(char *buf, int max_len) {
    read(recv_fd, buf, max_len);
    return 0;
}

