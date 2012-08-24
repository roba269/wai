#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <time.h>
#include <signal.h>
#include "common_define.h"
#include "simple_match.h"
#include "sandbox.h"
#include "db_wrapper.h"

const int BUF_LEN = 1024;
char trans[BUF_LEN*BUF_LEN];

static std::string exit_flag_2_str(int exit_type) {
    switch (exit_type) {
    case EXIT_NORMAL:
        return "The_opponent_exited_normally.";
    case EXIT_RF:
        return "The_opponent_called_restricted_function.";
    case EXIT_TLE:
        return "The_opponent_exceeded_time_limit.";
    case EXIT_MLE:
        return "The_opponent_exceeded_memory_limit.";
    case EXIT_RE:
        return "The_opponent_got_Runtime_Error.";
    default:
        return "The_opponent_exited_by_unknown_reason.";
    }
}

static std::string winner_to_str(int winner) {
    if (winner == 0)
        return "Draw.";
    else if (winner == 1)
        return "Player_1_win.";
    else
        return "Player_2_win.";
}

void SimpleMatch::Start() {
    signal(SIGPIPE, SIG_IGN);
    m_start_time = time(NULL);
    m_judge->Run(false);  /* No syscall restriction for judge */
    for (int i = 0 ; i < m_player.size() ; ++i) {
        m_player[i]->Run();
    }
    char buf[BUF_LEN], tmp_buf[BUF_LEN];
    trans[0] = 0;
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
            fprintf(stderr, "The judge try to read from player %d\n", src);
            memset(buf, 0, sizeof(buf));
            if (m_player[src]->Recv(buf, BUF_LEN-1) == 0) {
                int exit_type = m_player[src]->GetExitType();
                fprintf(stderr, "Player %d exited, type: %d\n", src, exit_type);
                m_winner = (1 - src) + 1;
                fprintf(stderr, "the winner is %d\n", m_winner);
                fprintf(stdout, "%d %s %s\n", m_winner,
                    winner_to_str(m_winner).c_str(),
                    exit_flag_2_str(exit_type).c_str());
                fflush(stdout);
                break;
            }
            fprintf(stderr, "The judge recv from %d: {%s}\n", src, buf);
            if (m_record.length()) m_record += ",";
            snprintf(tmp_buf, BUF_LEN, "%d:%s", src, buf);
            m_record += (std::string)tmp_buf;
            m_judge->Send(buf);
        } else if (buf[0] == '+') {
            fprintf(stdout, "%s\n", buf);
            fflush(stdout);
            // strcat(trans, buf+1);
        } else if (isdigit(buf[0])) {
            char res_str[BUF_LEN], reason[BUF_LEN];
            sscanf(buf, "%d %s %s", &m_winner, res_str, reason);
            fprintf(stderr, "the winner is %d\n", m_winner);
            fprintf(stdout, "%d %s %s\n", m_winner, res_str, reason);
            fflush(stdout);
            break;
        } else {
            assert(false);
        }
    }
    fprintf(stderr, "match main process exit.");
    m_end_time = time(NULL);
    if (_WriteToDatabase()) {
        fprintf(stderr, "write to database error\n");
    }
}

int SimpleMatch::_WriteToDatabase() {
/*
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
*/
    return 0;
}

