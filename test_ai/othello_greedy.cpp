#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <unistd.h>
using namespace std;

const int N = 8;
const int dir[][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},
    {1,-1},{1,0},{1,1}};

int board[N][N];

inline bool in(int x, int y) {
    return x >= 0 && x < N && y >= 0 && y < N;
}

int valid(int x, int y, int cur) {
    if (!in(x,y)) return 0;
    if (board[x][y] != 0) return 0;
    int tx, ty, ans = 0;
    for (int d = 0 ;  d < 8 ; d++) {
        tx = x + dir[d][0];
        ty = y + dir[d][1];
        if (in(tx,ty) && board[tx][ty] == 3 - cur) {
            while (true) {
                tx += dir[d][0];
                ty += dir[d][1];
                if (!in(tx,ty)) break;
                if (board[tx][ty] == cur) {
                    tx -= dir[d][0];
                    ty -= dir[d][1];
                    while (tx != x || ty != y) {
                        ++ans;
                        tx -= dir[d][0];
                        ty -= dir[d][1];
                    }
                    break;
                }
                else if (board[tx][ty] == 0)
                    break;
            }
        } 
    }
    return ans;
}

void put_chess(int x, int y, int cur) {
    board[x][y] = cur;
    int tx, ty;
    for (int d = 0 ; d < 8 ; ++d) {
        tx = x + dir[d][0];
        ty = y + dir[d][1];
        while (in(tx,ty) && board[tx][ty] == 3 - cur) {
            tx += dir[d][0];
            ty += dir[d][1];
        }
        if (in(tx,ty) && board[tx][ty] == cur) {
            tx -= dir[d][0];
            ty -= dir[d][1];
            while (tx != x || ty != y) {
                board[tx][ty] = cur;
                tx -= dir[d][0];
                ty -= dir[d][1];
            }
        }
    }
}

int main() {
    board[3][3] = board[4][4] = 1;
    board[3][4] = board[4][3] = 2;
    char buf[16];
    int flg;
    scanf("%s", buf);
    fprintf(stderr, "%d: I get {%s}\n", getpid(), buf);
    if (strcmp(buf, "first") == 0) {
        flg = 1;
    } else {
        flg = 2;
    }
    if (flg == 2) {
        int x, y;
        scanf("%d %d", &x, &y);
        put_chess(x, y, 3 - flg);
    }
    while (1) {
        int best = -1, bestx, besty;
        for (int i = 0 ; i < N ; i++)
            for (int j = 0 ; j < N ; j++) {
                if (board[i][j] != 0) continue;
                int tmp;
                if (tmp = valid(i, j, flg)) {
                    if (tmp > best) {
                        best = tmp;
                        bestx = i;
                        besty = j;
                    }
                }
            }
        if (best <= 0) {
          printf("-1 -1\n");  // can't move, output any (x, y)
          fflush(stdout);
        } else {
          printf("%d %d\n", bestx, besty);
          fflush(stdout);
          put_chess(bestx, besty, flg);
        }
        int x, y;
        scanf("%d %d", &x, &y);
        if (x != -1) {
          if (valid(x, y, 3-flg) <= 0) break;
          put_chess(x, y, 3-flg);
        }
    }
    return 0;
}

