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
    }
    void AddPlayer(Player *p) {
        m_players.push_back(p);
    }
    int GetPlayerCnt() {return m_players.size();}
    Player *GetPlayer(int idx) {return m_players[idx];}
    virtual int CheckWinner() = 0;
    virtual int MakeMove(Move *move) = 0;
    virtual int Start() = 0;
private:
    std::string m_name;
    std::vector<Player*> m_players;
};

#endif
