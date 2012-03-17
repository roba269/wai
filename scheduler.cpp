#include <cstdio>
#include <vector>
#include <set>
#include <cctype>
#include "mysql.h"
#include "db_wrapper.h"
#include "scheduler.h"

const int MAX_CMD_LEN = 512;

Scheduler* Scheduler::s_inst = NULL;

int Scheduler::_build_player_list(int game_type) {
    MYSQL *handle = DBWrapper::GetHandle();
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, MAX_CMD_LEN, "SELECT user_id,max(id) FROM main_app_submit WHERE game_type = %d GROUP BY user_id", game_type);
    mysql_query(handle, cmd);
    MYSQL_RES *mysql_res = mysql_store_result(handle);
    MYSQL_ROW row;
    int tmp_id = 0;
    while (row = mysql_fetch_row(mysql_res)) {
        int uid, sid;
        sscanf(row[0], "%d", &uid);
        sscanf(row[1], "%d", &sid);
        m_uid2sid[game_type][uid] = sid;
    }
    mysql_free_result(mysql_res);
    return 0;
}

static int GetUidFromSid(int sid) {
    int uid = -1;
    MYSQL *handle = DBWrapper::GetHandle();
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, MAX_CMD_LEN, "SELECT user_id FROM main_app_submit WHERE id=%d", sid);
    mysql_query(handle, cmd);
    MYSQL_RES *res = mysql_store_result(handle);
    MYSQL_ROW row;
    if (row = mysql_fetch_row(res)) {
        sscanf(row[0], "%d", &uid);
    }
    mysql_free_result(res);
    return uid;
}

int Scheduler::_build_matrix(int game_type) {
    MYSQL *handle = DBWrapper::GetHandle();
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, MAX_CMD_LEN, "SELECT id FROM main_app_match WHERE game_type = %d AND result != -1", game_type);
    mysql_query(handle, cmd);
    MYSQL_RES *mysql_res = mysql_store_result(handle);
    MYSQL_ROW row;
    while (row = mysql_fetch_row(mysql_res)) {
        int match_id;
        sscanf(row[0], "%d", &match_id);
        char cmd[MAX_CMD_LEN];
        std::vector<int> sid_vec;
        snprintf(cmd, MAX_CMD_LEN, "SELECT submit_id FROM main_app_match WHERE id = %d", match_id);
        mysql_query(handle, cmd);
        MYSQL_RES *mysql_res2 = mysql_store_result(handle);
        MYSQL_ROW row2;
        while (row2 = mysql_fetch_row(mysql_res2)) {
            int sid;
            sscanf(row[0], "%d", &sid);
            sid_vec.push_back(sid);
        }
        mysql_free_result(mysql_res2);
        assert(sid_vec.size() == 2);
        
        int u1 = GetUidFromSid(sid_vec[0]);
        int u2 = GetUidFromSid(sid_vec[1]);
        assert(u1 != -1 && u2 != -1);
        if (u1 > u2) {
            std::swap(u1,u2);
            std::swap(sid_vec[0], sid_vec[1]);
        }
        std::pair<int,int> pr = std::make_pair(u1,u2);
        m_mat[game_type][pr] = std::max(sid_vec[0], sid_vec[1]);
    }
    mysql_free_result(mysql_res);
}

void Scheduler::RegNewSub(int game_type, int uid, int sid) {
    assert(m_uid2sid[game_type].count(uid) == 0 ||
            sid > m_uid2sid[game_type][uid]);
    m_uid2sid[game_type][uid] = sid;
}

void Scheduler::RegNewMatch(int game_type, int u1, int u2, int sub1, int sub2)
{
    if (u1 > u2) {std::swap(u1,u2); std::swap(sub1,sub2);}
    std::pair<int,int> pr = std::make_pair(u1, u2);
    assert(m_mat[game_type].count(pr) == 0 ||
            m_mat[game_type][pr] < std::max(sub1, sub2));
    m_mat[game_type][pr] = std::max(sub1, sub2);
}

int Scheduler::InitScheduler(int game_type) {
    _build_player_list(game_type);
    _build_matrix(game_type);
    return 0;
}

int Scheduler::ArrangeMatch(int game_type, int &u1, int &u2, int &sub1, int &sub2)
{
    std::map<int,int>::iterator itr, itr2;
    for (itr = m_uid2sid[game_type].begin() ; itr != m_uid2sid[game_type].end() ; ++itr) {
        itr2 = itr;
        for (++itr2 ; itr2 != m_uid2sid[game_type].end() ; ++itr2) {
            u1 = itr->first;
            u2 = itr2->first;
            sub1 = itr->second;
            sub2 = itr2->second;
            if (m_mat[game_type][std::make_pair(u1, u2)] != std::max(sub1,sub2))
            {
                return 1;
            }
        } 
    }
    return 0;
}

