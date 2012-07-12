#ifndef HVC_MATCH_H
#define HVC_MATCH_H

#include <string>
#include <vector>
#include "common_define.h"

class Sandbox;

class HVCMatch {
    // Note: only support 1 human vs 1 computer currently
public:
    void InitMatch(MatchType type, int id, std::string name) {
        m_type = type;
        m_id = id;
        m_name = name;
    }
    void SetComputer(Sandbox *computer, int submit_id) {
        m_computer = computer;
        m_sid = submit_id;
    }
    void SetJudge(Sandbox *judge) {
        m_judge = judge;
    }
    void Start();
private:
    MatchType m_type;
    Sandbox *m_judge;
    Sandbox *m_computer;
    int m_sid;
    std::string m_record;
    int m_id;
    int m_winner;
    time_t m_start_time, m_end_time;
    std::string m_name;
};

#endif
