#include <cstdio>
#include <vector>
#include <set>
#include <cctype>
#include "mysql.h"
#include "db_wrapper.h"
#include "scheduler.h"

static void ParseList(const char *str, std::vector<int> &res) {
    res.clear();
    int tmp;
    for (int i = 0, tmp = 0 ; str[i] ; i++) {
        if (isdigit(str[i])) {
            tmp = tmp * 10 + str[i] - '0';
        } else if (tmp) {
            res.push_back(tmp);
            tmp = 0;
        }
    }
    if (tmp) res.push_back(tmp);
}

void Scheduler::_add_pair(int a, int b) {
    m_mat[a].insert(b);
    m_mat[b].insert(a);
}

int Scheduler::_build_player_list() {
    m_plist.clear();
    DBWrapper *db = DBWrapper::GetInstance();
    db->Query("SELECT uid FROM tab_submit WHERE matchid = 1 AND latest = 1");
    MYSQL_ROW row;
    int tmp_id = 0;
    while (row = db->FetchRow()) {
        int uid;
        sscanf(row[0], "%d", &uid);
        // assert(m_uid2id.count(uid) == 0);
        // m_uid2id[uid] = ++tmp_id;
        m_plist.push_back(uid);
    }
    db->FreeResult();
    return 0;
}

int Scheduler::_build_matrix() {
    DBWrapper *db = DBWrapper::GetInstance();
    db->Query("SELECT * FROM tab_match WHERE status=2");
    MYSQL_ROW row;
    while (row = db->FetchRow()) {
        int player_list[2];
        sscanf(row[2], "%d:%d", &player_list[0], &player_list[1]);
        _add_pair(player_list[0], player_list[1]);
        _add_pair(player_list[1], player_list[0]);
    }
    db->FreeResult();
}

int Scheduler::InitScheduler() {

}

int Scheduler::ArrangeMatch(int &p1, int &p2) {
    if (_build_player_list() != 0)
        return -1;
    if (_build_matrix() != 0)
        return -1;
    for (int i = 0 ; i < m_plist.size() ; ++i) {
        for (int j = 0 ; j < i ; ++j) {
            if (m_mat[m_plist[i]].count(m_plist[j])) continue;
            p1 = m_plist[i];
            p2 = m_plist[j];
            return 1;
        }
    }
    return 0;
}

