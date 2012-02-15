#ifndef SANDBOX_H
#define SANDBOX_H

#include <string>

class Sandbox {
public:
    Sandbox(std::string path);
    virtual ~Sandbox();
    virtual int Run();
    virtual int Send(char *buf);
    virtual int Recv(char *buf, int max_len);
private:
    std::string m_path;
    int send_fd, recv_fd;
};

#endif
