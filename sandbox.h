#ifndef SANDBOX_H
#define SANDBOX_H

#include <string>

const int MAX_BUF_LEN = 1024;

class Sandbox {
public:
    Sandbox(std::string path);
    virtual ~Sandbox();
    virtual int Run(bool is_rf = true, bool is_hvc = false);
    virtual int Send(char *buf);
    virtual int Recv(char *buf, int max_len, ExitFlagType &flag);
    virtual int GetTimeCost();
private:
    ExitFlagType _GetExitTypeAndTimeCost();
    int _RecvChar(char *buf, ExitFlagType &flag);
    void _InitSyscallSpec();
    std::string m_path;
    int send_fd, recv_fd, recv_info_fd;
    char m_buf[MAX_BUF_LEN];
    int m_idx, m_len;
    ExitFlagType m_exit_flag;
    int m_limit[512];
    int m_stat[512];
    int m_time_cost;
    int m_client_pid;
};

#endif
