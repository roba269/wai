#include <cstdlib>
#include <cstdio>
#include "match_renju.h"
#include "move.h"
#include "player.h"

#define ON_BOARD(x,y) ((x)>=0&&(x)<RENJU_SIZE&&(y)>=0&&(y)<RENJU_SIZE)
static const int dir[][2] = {
    {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
};

int MatchRenju::MakeMove(Move *move) {
    MoveRenju *mv = dynamic_cast<MoveRenju*>(move);
    int x = mv->x, y = mv->y, flg = mv->flg;
    if (!ON_BOARD(x,y) || m_board[x][y] != 0) {
        return -1;
    }
    m_board[x][y] = flg;
    return 0;
};

int MatchRenju::CheckWinner() {
    // TODO: no checking forbidden move 
    int i, j;
    for (i = 0 ; i < RENJU_SIZE ; ++i)
        for (j = 0 ; j < RENJU_SIZE ; ++j) {
            if (m_board[i][j]) continue;
            for (int d = 0 ; d < 8 ; d++) {
                int cnt = 1, ti, tj;
                ti = i + dir[d][0];
                tj = j + dir[d][1];
                while (ON_BOARD(ti, tj) && m_board[ti][tj] == m_board[i][j]) {
                    if (++cnt == 5) return m_board[i][j];
                    ti += dir[d][0];
                    tj += dir[d][1];
                }
            }
        }
    return 0;
}

int MatchRenju::Start() {
    char buf[32];
    int flg = 0;
    Player *p[2];
    p[0] = GetPlayer(0);
    p[1] = GetPlayer(1);
    p[0]->SendMessage("First");
    p[1]->SendMessage("Second");
    while (1) {
        p[flg]->RecvMessage(buf, 30);
        MoveRenju tm;
        sscanf(buf, "%d %d", &tm.x, &tm.y);
        tm.flg = flg + 1;
        MakeMove(&tm);
        if (CheckWinner()) break;
        p[1-flg]->SendMessage(buf);
        flg = 1 - flg;
    }
}

