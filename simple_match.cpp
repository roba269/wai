#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include "common_define.h"
#include "simple_match.h"
#include "sandbox.h"
#include "db_wrapper.h"

const int BUF_LEN = 1024;

void SimpleMatch::Start() {
    m_start_time = time(NULL);
    m_judge->Run();
    for (int i = 0 ; i < m_player.size() ; ++i) {
        m_player[i]->Run();
    }
    char buf[BUF_LEN], tmp_buf[BUF_LEN];
    while (1) {
        fprintf(stderr, "waiting for the judge speaking\n");
        memset(buf, 0, sizeof(buf));
        m_judge->Recv(buf, BUF_LEN-1);
        fprintf(stderr, "The judge said: {%s}\n", buf);
        if (buf[0] == '>') {
            int dst = buf[1] - '1';
            m_player[dst]->Send(buf+4);
        } else if (buf[0] == '<') {
            int src = buf[1] - '1';
            memset(buf, 0, sizeof(buf));
            if (m_player[src]->Recv(buf, BUF_LEN-1) == 0) {
                fprintf(stderr, "Player %d exited, type: %d\n",
                        src, m_player[src]->GetExitType());
                break;
            }
            fprintf(stderr, "The judge recv from %d: {%s}\n", src, buf);
            if (m_record.length()) m_record += ",";
            snprintf(tmp_buf, BUF_LEN, "%d:%s", src, buf);
            m_record += (std::string)tmp_buf;
            m_judge->Send(buf);
        } else if (isdigit(buf[0])) {
            sscanf(buf, "%d", &m_winner);
            fprintf(stderr, "the winner is %d\n", m_winner);
            break;
        } else {
            assert(false);
        }
    }
    m_end_time = time(NULL);
    if (_WriteToDatabase()) {
        fprintf(stderr, "write to database error\n");
    }
}

int SimpleMatch::_WriteToDatabase() {
    char cmd[BUF_LEN];
    snprintf(cmd, BUF_LEN, "INSERT INTO main_app_match (game_type, result, start_time, end_time, player_cnt) VALUES (\"%s\", %d, FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), %d)", "RENJU", m_winner, m_start_time, m_end_time, m_player.size());
    MYSQL *handle = DBWrapper::GetHandle();
    fprintf(stderr, "%s\n", cmd);
    mysql_query(handle, cmd);
    int match_id = mysql_insert_id(handle);
    for (int i = 0 ; i < m_sid.size() ; ++i) {
        snprintf(cmd, BUF_LEN, "INSERT INTO main_app_match_players "
            "(submit_id, match_id) VALUES (%d,%d)", m_sid[i], match_id);
        fprintf(stderr, "%s\n", cmd);
        mysql_query(handle, cmd);
    }
    snprintf(cmd, BUF_LEN, "%s/%d.txt", RECORD_PREFIX, match_id);
    FILE *fp = fopen(cmd, "w");
    fprintf(fp, "%s\n", m_record.c_str());
    fclose(fp);
    return 0;
}

