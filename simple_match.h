#ifndef SIMPLE_MATCH_H
#define SIMPLE_MATCH_H

#include <string>
#include <vector>
#include "common_define.h"

class Sandbox;

class SimpleMatch {
public:
    void InitMatch(MatchType type, int id, std::string name) {
        m_type = type;
        m_name = name;
        m_id = id;
    }
    void AddPlayer(Sandbox *player, int submit_id) {
        m_player.push_back(player);
        m_sid.push_back(submit_id);
    }
    void SetJudge(Sandbox *judge) {
        m_judge = judge;
    }
    void Start();
private:
    int _WriteToDatabase();

    MatchType m_type;
    std::string m_name;
    std::vector<Sandbox*> m_player;
    std::vector<int> m_sid;
    Sandbox *m_judge;
    int m_id;
    int m_winner;
    time_t m_start_time, m_end_time;
};

#endif
