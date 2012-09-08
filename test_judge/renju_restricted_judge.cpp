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
    int i, j, tot_cnt = 0;
    for (i = 0 ; i < R ; i++)
        for (j = 0 ; j < C ; j++) {
            if (board[i][j] == 0) continue;
            ++tot_cnt;
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
    if (tot_cnt == R * C) return 3; // Draw
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

int check_five(int x, int y, int d, int bx = -1, int by = -1) {
  if (board[x][y]) {
    fprintf(stderr, "error when check five\n");
    return 0;
  }
  int cnt = 1, b_flg = 0;
  for (int flg = 0 ; flg < 2 ; ++flg) {
    int tmp;
    if (flg == 0) tmp = 1;
    else tmp = -1;
    int tx, ty;
    tx = x + tmp * dir[d][0];
    ty = y + tmp * dir[d][1];
    while (IN(tx,ty) && board[tx][ty] == 1) {
      if (tx == bx && ty == by) b_flg = 1;
      tx += tmp * dir[d][0];
      ty += tmp * dir[d][1];
      ++cnt;
    }
  }
  if (cnt == 5) {
    if (bx != -1 && b_flg == 0)
      return 0;
    else
      return 1;
  }
  return 0;
}

int check_four(int x, int y, int d) {
  if (board[x][y]) {
    fprintf(stderr, "error when check four\n");
    return 0;
  }
  board[x][y] = 1;
  for (int flg = 0 ; flg < 2 ; ++flg) {
    int tmp;
    if (flg == 0) tmp = 1;
    else tmp = -1;
    int tx, ty;
    tx = x + tmp * dir[d][0];
    ty = y + tmp * dir[d][1];
    int dis = 1;
    while (IN(tx,ty) && dis <= 4) {
      if (board[tx][ty] == 2) break;
      if (board[tx][ty] == 0 && check_five(tx, ty, d, x, y)) {
        board[x][y] = 0;
        return 1;
      }
      tx += tmp * dir[d][0];
      ty += tmp * dir[d][1];
      ++dis;
    }
  }
  board[x][y] = 0;
  return 0;
}

int check_open_four(int x, int y, int d) {
  if (board[x][y] != 0) {
    fprintf(stderr, "check_open_four failed.");
    return 0;
  }
  int flg = 1;
  for (int tx = 0 ; tx < R ; ++tx)
    for (int ty = 0 ; ty < C ; ++ty) {
      if (x == tx && y == ty) continue;
      if (board[tx][ty] != 0) continue;
      board[tx][ty] = 2;
      if (!check_four(x, y, d)) {
        board[tx][ty] = 0;
        flg = 0;
        goto out;
      }
      board[tx][ty] = 0;
    }
out:
  return flg;
}

int check_open_three(int x, int y, int d) {
  board[x][y] = 1;
  for (int flg = 0 ; flg < 2 ; ++flg) {
    int tmp;
    if (flg == 0) tmp = 1;
    else tmp = -1;
    int tx, ty;
    tx = x + tmp * dir[d][0];
    ty = y + tmp * dir[d][1];
    int dis = 1;
    while (IN(tx,ty) && dis <= 4) {
      if (board[tx][ty] == 2) break;
      if (board[tx][ty] == 0 && check_open_four(tx, ty, d)) {
        board[x][y] = 0;
        return 1;
      }
      tx += tmp * dir[d][0];
      ty += tmp * dir[d][1];
      ++dis;
    }
  } 
  board[x][y] = 0;
  return 0;
}

int check_restriction(int x, int y, char *str) {
  int cnt[4][2];
  int cnt3 = 0, cnt4 = 0;
  memset(cnt, 0, sizeof(cnt));
  for (int d = 0 ; d < 4 ; ++d) {
    for (int flg = 0 ; flg < 2 ; flg++) {
      int tmp;
      if (flg == 0) tmp = 1;
      else tmp = -1;
      int tx, ty;
      tx = x + tmp * dir[d][0];
      ty = y + tmp * dir[d][1];
      while (IN(tx,ty) && board[tx][ty] == 1) {
        tx += tmp * dir[d][0];
        ty += tmp * dir[d][1];
        ++cnt[d][flg];
      }
    }
    if (cnt[d][0] + cnt[d][1] >= 5) {
      strcpy(str, "six-in-a-row");
      return 1;
    }
  }
  for (int d = 0 ; d < 4 ; ++d) {
    if (check_four(x, y, d)) ++cnt4;
    else if (check_open_three(x, y, d)) ++cnt3;
  }
  if (cnt4 >= 2) {
    strcpy(str, "four-four");
    return 1;
  }
  if (cnt3 >= 2) {
    strcpy(str, "three-three");
    return 1;
  }
  return 0;
}

inline string get_result_str(int side) {
  if (side == 0) return "Draw";
  if (side == 1) return "Player_1_win";
  return "Player_2_win";
}

bool allow[3] = {false, false, false};

int main(int argc, char **argv) {
    for (int i = 1 ; i < argc ; ++i) {
      int tmp;
      sscanf(argv[1], "%d", &tmp);
      allow[tmp] = true;
    }
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
start:
        printf("<%d\n", cur);
        fflush(stdout);
        int x, y;
        fgets(tmp_buf, BUF_LEN, stdin);
        sscanf(tmp_buf, "%d %d", &x, &y);
        if (!valid(x,y)) {
            if (allow[cur]) {
              printf("*\n");
              fflush(stdout);
              goto start;
            }
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
        char tmp_buf[64];
        if (cur == 1 && check_restriction(x, y, tmp_buf)) {
          printf("%d %s Black_restricted_move:%s.\n", 3-cur,
            get_result_str(3-cur).c_str(), tmp_buf);
          fflush(stdout);
          return 0;
        }
        board[x][y] = cur;
        output_board();
        int w;
        if ((w = get_winner())) {
            if (w == 3) {
                printf("0 draw Board_is_full.\n");
                fflush(stdout);
            } else {
                printf("%d %s %s\n", w, get_result_str(w).c_str(),
                  get_result_str(w).c_str());
                fflush(stdout);
            }
            return 0;
        }
        printf(">%d: %d %d\n", 3-cur, x, y);
        fflush(stdout);
        cur = 3 - cur;
    }
    return 0;
}

