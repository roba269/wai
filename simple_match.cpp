#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "simple_match.h"
#include "sandbox.h"

const int BUF_LEN = 1024;

void SimpleMatch::Start() {
    m_judge->Run();
    for (int i = 0 ; i < m_player.size() ; ++i) {
        m_player[i]->Run();
    }
    char buf[BUF_LEN];
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
            m_player[src]->Recv(buf, BUF_LEN-1);
            fprintf(stderr, "The judge recv from %d: %s\n", src, buf);
            m_judge->Send(buf);
        } else if (isdigit(buf[0])) {
            sscanf(buf, "%d", &m_winner);
            fprintf(stderr, "the winner is %d\n", m_winner);
            if (_WriteToDatabase()) {
                fprintf(stderr, "write to database error\n");
            }
            break;
        } else {
            assert(false);
        }
    }
}

int SimpleMatch::_WriteToDatabase() {
    return 0;
}

