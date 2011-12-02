#ifndef MATCH_H
#define MATCH_H

#include <vector>
#include <string>

class Player;
class Move;

class Match {
public:
    void InitMatch(std::string name) {
        m_name = name;
        m_players.clear();
        m_winner = -1;
    }
    void AddPlayer(Player *p) {
        m_players.push_back(p);
    }
    int GetPlayerCnt() {return m_players.size();}
    Player *GetPlayer(int idx) {return m_players[idx];}
    virtual int CheckWinner() = 0;
    virtual int MakeMove(Move *move) = 0;
    virtual int Start() = 0;
    int SendMsg(int player_idx, const char *msg);
    int RecvMsg(int player_idx, char *msg, int maxlen, int &exit_idx);
    int KillPlayer(int player_idx);
    void SetWinner(int k) {m_winner = k;}
    int GetWinner() {return m_winner;}
private:
    std::string m_name;
    std::vector<Player*> m_players;
    int m_winner;
};

#endif
