#ifndef PLAYER_H
#define PLAYER_H

#include <string>

enum PlayerType {
    PLAYER_COMPUTER,
    PLAYER_HUMAN
};

class Player {
public:
    virtual ~Player() {};
    void SetType(PlayerType tp) {m_type = tp;}
    PlayerType GetType() {return m_type;}
    void SetName(std::string name) {m_name = name;}
    std::string GetName() {return m_name;}
    virtual void SendMessage(const char *msg) = 0;
    virtual void RecvMessage(char *msg, int maxlen) = 0;
private:
    PlayerType m_type;
    std::string m_name;
};

class PlayerComputer : Player {
public:
    // find the exe with name ai_name, fork a new process
    // build the pipe
    int LoadAI(std::string ai_name);
    void SendMessage(const char *msg);
    void RecvMessage(char *msg, int maxlen);
private:
    int m_infd, m_outfd;
};

class PlayerHuman : Player {

};

#endif
