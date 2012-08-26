#include <time.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "common_define.h"
#include "hvc_match.h"
#include "sandbox.h"

const int BUF_LEN = 1024;

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

void HVCMatch::Start() {
    m_start_time = time(NULL);
    m_judge->Run(false);
    m_computer->Run();
    char buf[BUF_LEN], tmp_buf[BUF_LEN];
    while (1) {
        memset(buf, 0, sizeof(buf));
        ExitFlagType tmp;
        if (m_judge->Recv(buf, BUF_LEN-1, tmp) == 0) {
            fprintf(stderr, "The judge crashed.");
            break;
        }
        fprintf(stderr, "The judge said {%s}\n", buf);
        if (buf[0] == '>') {
            int dst = buf[1] - '1';
            if (dst == 0) {
                // to human: display the computer's move
                printf("%s\n", buf+4);
                fflush(stdout); // neccessary?
            } else if (dst == 1) {
                // computer
                m_computer->Send(buf+4);
            } else assert(false);
        } else if (buf[0] == '<') {
            int src = buf[1] - '1';
            if (src == 0) {
                // from human: request move from browser
                // scanf("%s", tmp_buf);
                fgets(tmp_buf, BUF_LEN-1, stdin);
                int tmp_len = strlen(tmp_buf);
                if (tmp_buf[tmp_len-1] == '\n') tmp_buf[tmp_len-1] = 0;
                m_judge->Send(tmp_buf);
            } else if (src == 1) {
                ExitFlagType exit_type;
                if (m_computer->Recv(buf, BUF_LEN-1, exit_type) == 0) {
                    fprintf(stderr, "Computer exited, type: %d\n", (int)exit_type);
                    m_winner = (1 - src) + 1;
                    fprintf(stderr, "the winner is %d\n", m_winner);
                    printf(": %d %s %s\n", m_winner,
                      winner_to_str(m_winner).c_str(),
                      exit_flag_2_str(exit_type).c_str());
                    fflush(stdout);
                    break;
                }
                m_judge->Send(buf);
            } else assert(false);
        } else if (isdigit(buf[0])) {
            char res_str[BUF_LEN], reason[BUF_LEN];
            sscanf(buf, "%d %s %s", &m_winner, res_str, reason);
            printf(": %d %s %s\n", m_winner, res_str, reason);
            fflush(stdout);
            break;
        } // ignore '+'
    }
    m_end_time = time(NULL);
};

