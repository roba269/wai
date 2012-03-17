#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <set>
#include <map>
#include "common_define.h"

class Scheduler {
public:
    virtual ~Scheduler() {};
    static Scheduler* GetInstance() {
        if (s_inst == NULL) {
            s_inst = new Scheduler();
        }
        return s_inst;
    }
    int InitScheduler(int game_type);
    virtual int ArrangeMatch(int game_type, int &u1, int &u2, int &sub1, int &sub2);
    virtual void RegNewSub(int game_type, int uid, int sid);
    virtual void RegNewMatch(int game_type, int u1, int u2, int sub1, int sub2);
private:
    int _build_player_list(int game_type);
    int _build_matrix(int game_type);
    // void _add_pair(int a, int b);
    std::map<std::pair<int,int>, int> m_mat[MAX_GAME_TYPE];
    // std::vector<int> m_plist, m_sub_list;
    std::map<int, int> m_uid2sid[MAX_GAME_TYPE];
    static Scheduler* s_inst;
};

#endif
