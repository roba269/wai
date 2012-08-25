#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <string>
using namespace std;

const int BUF_LEN = 128;
int board[16][16];
const int R = 15;
const int C = 15;
const int dir[][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

#define IN(x,y) ((x)>=0&&(x)<R&&(y)>=0&&(y)<C)

int valid(int x, int y) {
    if (x < 0 || x >= R || y < 0 || y >= C)
        return 0;
    if (board[x][y]) return 0;
    return 1;
}

int get_winner() {
    int i, j;
    for (i = 0 ; i < R ; i++)
        for (j = 0 ; j < C ; j++) {
            if (board[i][j] == 0) continue;
            for (int d = 0 ; d < 8 ; ++d) {
                int ti = i, tj = j, cnt = 1;
                while (1) {
                    ti += dir[d][0];
                    tj += dir[d][1];
                    if (IN(ti,tj) && board[ti][tj] == board[i][j]) {
                        if (++cnt == 5) return board[i][j];
                    } else break;
                }
            }
        }
    return 0;
}

void output_board() {
    char buf[32];
    int i, j;
    for (i = 0 ; i < R ; i++) {
        for (j = 0 ; j < C ; j++) {
            if (board[i][j] == 0) buf[j] = '-';
            else if (board[i][j] == 1) buf[j] = 'O';
            else buf[j] = 'X';
        }
        buf[j] = 0;
        fprintf(stderr, "%s\n", buf);
    }
}

inline string get_result_str(int side) {
  if (side == 0) return "Draw";
  if (side == 1) return "Player_1_win";
  return "Player_2_win";
}

int main() {
    fprintf(stderr, "%d: I am the judge\n", getpid());
    memset(board, 0, sizeof(board));
    printf(">1: first\n");
    fflush(stdout);
    fprintf(stderr, "%d: I said >1: first\n", getpid());
    printf(">2: second\n");
    fflush(stdout);
    fprintf(stderr, "%d: I said >2: second\n", getpid());
    int cur = 1;
    char tmp_buf[BUF_LEN+10];
    while (1) {
        printf("<%d\n", cur);
        fflush(stdout);
        int x, y;
        fgets(tmp_buf, BUF_LEN, stdin);
        sscanf(tmp_buf, "%d %d", &x, &y);
        if (!valid(x,y)) {
            if (cur == 1)
              printf("%d %s Black_make_invalid_move.\n", 3-cur,
                get_result_str(3-cur).c_str());
            else
              printf("%d %s White_make_invalid_move.\n", 3-cur,
                get_result_str(3-cur).c_str());
            fflush(stdout);
            return 0;
        }
        printf("+%d %d %d\n", cur, x, y);
        fflush(stdout);
        board[x][y] = cur;
        output_board();
        int w;
        if ((w = get_winner())) {
            printf("%d %s %s\n", w, get_result_str(w).c_str(), get_result_str(w).c_str());
            fflush(stdout);
            return 0;
        }
        printf(">%d: %d %d\n", 3-cur, x, y);
        fflush(stdout);
        cur = 3 - cur;
    }
    return 0;
}

