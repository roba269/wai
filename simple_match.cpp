#include <cassert>
#include <cstdio>
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
        m_judge->Recv(buf, BUF_LEN-1);
        if (buf[0] == '>') {
            int dst = buf[1] - '0';
            m_player[dst]->Send(buf+4);
        } else if (buf[0] == '<') {
            int src = buf[1] - '0';
            m_player[src]->Recv(buf, BUF_LEN-1);
            m_judge->Send(buf);
        } else if (isdigit(buf[0])) {
            sscanf(buf, "%d", &m_winner);
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

