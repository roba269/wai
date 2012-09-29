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

static std::string exit_flag_2_str(ExitFlagType exit_type) {
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
    for (unsigned int i = 0 ; i < m_player.size() ; ++i) {
        m_player[i]->Run();
    }
    char buf[BUF_LEN], tmp_buf[BUF_LEN];
    trans[0] = 0;
    while (1) {
        fprintf(stderr, "waiting for the judge speaking\n");
        memset(buf, 0, sizeof(buf));
        ExitFlagType tmp;
        if (m_judge->Recv(buf, BUF_LEN-1, tmp) == 0) {
            fprintf(stderr, "The judge crashed.\n");
            break;
        }
        fprintf(stderr, "The judge said: {%s}\n", buf);
        if (buf[0] == '>') {
            int dst = buf[1] - '1';
            m_player[dst]->Send(buf+4);
        } else if (buf[0] == '<') {
            int src = buf[1] - '1';
            fprintf(stderr, "The judge try to read from player %d\n", src);
            memset(buf, 0, sizeof(buf));
            ExitFlagType exit_type;
            if (m_player[src]->Recv(buf, BUF_LEN-1, exit_type) == 0) {
                fprintf(stderr, "Player %d exited, type: %d\n", src, (int)exit_type);
                m_winner = (1 - src) + 1;
                fprintf(stderr, "the winner is %d\n", m_winner);
                fprintf(stdout, "%d %s %s %d %d\n", m_winner,
                    winner_to_str(m_winner).c_str(),
                    exit_flag_2_str(exit_type).c_str(),
                    m_player[0]->GetTimeCost(),
                    m_player[1]->GetTimeCost());
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
            int p1_time = m_player[0]->GetTimeCost();
            int p2_time = m_player[1]->GetTimeCost();
            fprintf(stdout, "%d %s %s %d %d\n", m_winner, res_str, reason, p1_time, p2_time);
            fflush(stdout);
            break;
        } else {
            assert(false);
        }
    }
    fprintf(stderr, "match main process exit.\n");
    m_end_time = time(NULL);
}

