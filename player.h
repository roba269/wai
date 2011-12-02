#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "common_define.h"

enum PlayerType {
    PLAYER_COMPUTER,
    PLAYER_HUMAN
};

class Player {
public:
    Player();
    virtual ~Player() {};
    void SetType(PlayerType tp) {m_type = tp;}
    PlayerType GetType() {return m_type;}
    void SetName(std::string name) {m_name = name;}
    std::string GetName() {return m_name;}
    void SetID(int id) {m_id = id;}
    int GetID() {return m_id;}
    int GetInputFd() {return m_infd;}
    int GetOutputFd() {return m_outfd;}
    int GetInfoFd() {return m_info_fd;}
    void SetInputFd(int fd) {m_infd = fd;}
    void SetOutputFd(int fd) {m_outfd = fd;}
    void SetInfoFd(int fd) {m_info_fd = fd;}
    // virtual void SendMessage(const char *msg) = 0;
    // virtual void RecvMessage(char *msg, int maxlen) = 0;
    virtual void Kill() = 0;
private:
    PlayerType m_type;
    std::string m_name;
    int m_id;
    int m_infd, m_outfd, m_info_fd;
};

class PlayerComputer : public Player {
public:
    PlayerComputer();
    // find the exe with name ai_name, fork a new process
    // build the pipe
    int LoadAI(std::string ai_name, int id);
    // void SendMessage(const char *msg);
    // void RecvMessage(char *msg, int maxlen);
    void Kill();
private:
    void InitSyscallSpec();
    pid_t m_sandbox_pid;
    int m_limit[512];
    int m_stat[512];
    ExitFlagType m_exit_flag;
};

class PlayerHuman : public Player {

};

#endif
