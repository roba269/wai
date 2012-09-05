#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <string>
using namespace std;

const int BUF_LEN = 128;
const int R = 10;
const int C = 9;

enum ChessType {
  B_SHUAI = 1,
  B_SHI,
  B_XIANG,
  B_JU,
  B_MA,
  B_PAO,
  B_ZU,
  R_SHUAI = 11,
  R_SHI,
  R_XIANG,
  R_JU,
  R_MA,
  R_PAO,
  R_ZU,
};

const int dir_ma[][2] = {{-2,-1},{-2,1},{-1,-2},{1,-2},
  {-1,2},{1,2},{2,-1},{2,1}};
const int dir_mt[][2] = {{-1,0},{-1,0},{0,-1},{0,-1},
  {0,1},{0,1},{1,0},{1,0}};
const int dir_xiang[][2] = {{-2,-2},{-2,2},{2,-2},{2,2}};
const int dir_xy[][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};

int bd[R][C];

void init_board() {
  memset(bd, 0, sizeof(bd));
  bd[0][0] = bd[0][8] = B_JU;
  bd[0][1] = bd[0][7] = B_MA;
  bd[0][2] = bd[0][6] = B_XIANG;
  bd[0][3] = bd[0][5] = B_SHI;
  bd[0][4] = B_SHUAI;
  bd[2][1] = bd[2][7]  = B_PAO;
  bd[3][0] = bd[3][2] = bd[3][4] = bd[3][6] = bd[3][8] = B_ZU;
  for (int i = 0 ; i < 5 ; i++)
    for (int j = 0 ; j < C ; j++)
      if (bd[i][j] != 0)
        bd[R-1-i][j] = bd[i][j] + 10;
}

void move_chess(int cur, int x1, int y1, int x2, int y2) {
  bd[x2][y2] = bd[x1][y1];
  bd[x1][y1] = 0;
}

int get_winner() {
  int flg_b = 0, flg_r = 0;
  for (int i = 0 ; i < R ; i++)
      for (int j = 0 ; j < C ; j++)
        if (bd[i][j] == B_SHUAI) flg_b = 1; 
        else if (bd[i][j] == R_SHUAI) flg_r = 1;
  if (!flg_b) return 1;
  if (!flg_r) return 2;
  return 0;
}

inline int in(int x, int y) {
  return x >= 0 && x < R && y >= 0 && y < C;
}

inline int in_nine(int cur, int x, int y) {
  if (cur == 1) {
    return x >= 7 && x <= 9 && y >= 3 && y <= 5;
  } else {
    return x >= 0 && x <= 2 && y >= 3 && y <= 5;
  }
}

inline int in_side(int cur, int x, int y) {
  if (cur == 1) {
    return x >= 5 && x <= 9;
  } else {
    return x >= 0 && x <= 4;
  }
}

inline int has_chess(int cur, int x, int y) {
  if (cur == 1) return bd[x][y] >= R_SHUAI && bd[x][y] <= R_ZU;
  if (cur == 2) return bd[x][y] >= B_SHUAI && bd[x][y] <= B_ZU;
  return 0;
}

int check_shuai(int cur, int x1, int y1, int x2, int y2, char *str)
{
  if (!in_nine(cur, x2, y2)) {
    if (in_nine(3-cur, x2, y2) && y1 == y2 &&
          (bd[x2][y2] == B_SHUAI || bd[x2][y2] == R_SHUAI)) {
      int flg = 1;
      for (int t = min(x1,x2)+1 ; t <= max(x1,x2)-1 ; ++t) {
        if (bd[t][y1] != 0) {flg = 0; break;}
      } 
      if (flg) {
        // two shai face each other
        return 1;
      }
    }
    strcpy(str, "Shuai_moves_outside_the_nine-zone.");
    return 0;
  }
  int dis = abs(x1-x2) + abs(y1-y2);
  if (dis != 1) {
    strcpy(str, "Shuai_moves_too_long.");
    return 0;
  }
  return 1;
}

int check_shi(int cur, int x1, int y1, int x2, int y2, char *str)
{
  if (!in_nine(cur, x2, y2)) {
    strcpy(str, "Shi_moves_outside_the_nine-zone.");
    return 0;
  }
  if (abs(x1-x2) != 1 || abs(y1-y2) != 1) {
    strcpy(str, "Shi_invalid_move.");
    return 0;
  }
  return 1;
}

int check_xiang(int cur, int x1, int y1, int x2, int y2, char *str)
{
  if (!in_side(cur, x2, y2)) {
    strcpy(str, "Xiang_moves_outside_own_side_zone.");
    return 0;
  }
  int flg_x = 0;
  for (int d = 0 ; d < 4 ; d++) {
    int tmpx = x1 + dir_xiang[d][0];
    int tmpy = y1 + dir_xiang[d][1];
    if (tmpx == x2 && tmpy == y2) {
      int yanx = x1 + dir_xy[d][0];
      int yany = y1 + dir_xy[d][1];
      if (bd[yanx][yany] == 0)
        flg_x = 1;
      break;
    }
  }
  if (!flg_x) {
    strcpy(str, "Xiang_invalid_move.");
    return 0;
  }
  return 1;
}

int check_ju(int cur, int x1, int y1, int x2, int y2, char *str)
{
  if (x1 == x2) {
    for (int t = min(y1,y2) + 1 ; t <= max(y1,y2)-1 ; ++t) {
      if (bd[x1][t] != 0) {
        strcpy(str, "There_are_pieces_on_Ju_path.");
        return 0;
      }
    }
  } else if (y1 == y2) {
    for (int t = min(x1,x2)+1 ; t <= max(x1,x2)-1 ; ++t) {
      if (bd[t][y1] != 0) {
        strcpy(str, "There_are_pieces_on_Ju_path.");
        return 0;
      }
    }
  } else {
    strcpy(str, "Ju_must_move_in_a_line.");
    return 0;
  }
  return 1;
}

int check_ma(int cur, int x1, int y1, int x2, int y2, char *str)
{
  int flg_ma = 0;
  for (int d = 0 ; d < 8 ; d++) {
    int tmpx = x1 + dir_ma[d][0];
    int tmpy = y1 + dir_ma[d][1];
    if (tmpx == x2 && tmpy == y2) {
      int tuix = x1 + dir_mt[d][0];
      int tuiy = y1 + dir_mt[d][1];
      if (bd[tuix][tuiy] == 0) {
        flg_ma = 1;
        break;
      }
    }
  }
  if (!flg_ma) {
    strcpy(str, "Ma_invalid_move.");
    return 0;
  }
  return 1;
}

int check_pao(int cur, int x1, int y1, int x2, int y2, char *str)
{
  int block_cnt = 0;
  if (x1 == x2) {
    for (int t = min(y1,y2)+1 ; t <= max(y1,y2)-1 ; ++t) {
      if (bd[x1][t] != 0) ++block_cnt;
    }
  } else if (y1 == y2) {
    for (int t = min(x1,x2)+1 ; t <= max(x1,x2)-1 ; ++t) {
      if (bd[t][y1] != 0) ++block_cnt;
    }
  } else {
    strcpy(str, "Pao_must_move_in_a_line.");
    return 0;
  }
  if (block_cnt == 0) {
    // no block
    if (bd[x2][y2] != 0) {
      strcpy(str, "Pao_cannot_move_to_a_taken_pos.");
      return 0;
    }
  } else if (block_cnt == 1) {
    if (!has_chess(3-cur, x2, y2)) {
      strcpy(str, "Pao_move_over_1_piece,_and_must_capture_an_enemy_piece.");
      return 0;
    }
  } else {
    strcpy(str, "Pao_move_over_more_than_1_piece.");
    return 0;
  }
  return 1;
}

int check_zu(int cur, int x1, int y1, int x2, int y2, char *str)
{
  int dis = abs(x1-x2) + abs(y1-y2);
  if (dis != 1) {
    strcpy(str, "Zu_move_too_long.");
    return 0;
  }
  if (cur == 1 && x1 < x2) {
    strcpy(str, "Zu_cannot_move_backward.");
    return 0;
  }
  if (cur == 2 && x1 > x2) {
    strcpy(str, "Zu_cannot_move_backward.");
    return 0;
  }
  if (in_side(cur, x1, y1)) {
    // not pass the river
    if (y1 != y2) {
      strcpy(str, "Not-passing-river-Zu_only_move_forward.");
      return 0;
    }
  }
  return 1;
}

int valid(int cur, int x1, int y1, int x2, int y2, char *str) {
  if (!in(x1, y1) || !in(x2, y2)) {
    strcpy(str, "Position_outside_the_board.");
    return 0;
  }
  if (!has_chess(cur, x1, y1)) {
    strcpy(str, "No_chess_of_own_side_on_start_pos.");
    return 0;
  }
  if (has_chess(cur, x2, y2)) {
    strcpy(str, "Has_chess_of_own_side_on_target_pos.");
    return 0;
  }
  if (x1 == x2 && y1 == y2) {
    strcpy(str, "Identical_start_and_target_pos.");
    return 0;
  }
  int chess = bd[x1][y1];
  if (cur == 2) chess += 10;
  switch (chess) {
  case R_SHUAI:
    return check_shuai(cur, x1, y1, x2, y2, str); 
  case R_SHI:
    return check_shi(cur, x1, y1, x2, y2, str); 
  case R_XIANG:
    return check_xiang(cur, x1, y1, x2, y2, str); 
  case R_JU:
    return check_ju(cur, x1, y1, x2, y2, str); 
  case R_MA:
    return check_ma(cur, x1, y1, x2, y2, str); 
  case R_PAO:
    return check_pao(cur, x1, y1, x2, y2, str); 
  case R_ZU:
    return check_zu(cur, x1, y1, x2, y2, str); 
  }
  strcpy(str, "Invalid_chess_type");
  return 0;
}

int can_move(int cur) {
  char buf[128];
  for (int x1 = 0 ; x1 < R ; ++x1)
    for (int y1 = 0 ; y1 < C ; ++y1) {
      if (!has_chess(cur, x1, y1))
        continue;
      for (int x2 = 0 ; x2 < R ; ++x2)
        for (int y2 = 0 ; y2 < C ; ++y2) {
          if (valid(cur, x1, y1, x2, y2, buf))
            return 1;
        }
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
    sscanf(argv[i], "%d", &tmp);
    allow[tmp] = true;
  }
  char buf[128];
  init_board();
  printf(">1: first\n");
  fflush(stdout);
  printf(">2: second\n");
  fflush(stdout);
  int cur = 1;
  char tmp_buf[BUF_LEN+10];
  while (1) {
    if (!can_move(cur)) {
      printf("%d %s The_opponent_cannot_move.\n", 3-cur,
        get_result_str(3-cur).c_str());
      fflush(stdout);
      break;
    }
start:
    printf("<%d\n", cur);
    fflush(stdout);
    int x1, y1, x2, y2;
    fgets(tmp_buf, BUF_LEN, stdin);
    sscanf(tmp_buf, "%d %d %d %d", &x1, &y1, &x2, &y2);
    if (!valid(cur, x1, y1, x2, y2, buf)) {
      if (allow[cur]) {
        printf("*\n");
        fflush(stdout);
        goto start;
      }
      printf("%d %s The_opponent_makes_invalid_move:%s.\n",
        3-cur, get_result_str(3-cur).c_str(), buf);
      fflush(stdout);
      break;
    }
    printf("+%d %d %d %d %d\n", cur, x1, y1, x2, y2);
    fflush(stdout);
    move_chess(cur, x1, y1, x2, y2);
    printf(">%d: %d %d %d %d\n", 3-cur, x1, y1, x2, y2);
    int w;
    if ((w = get_winner())) {
      printf("%d %s %s\n", w, get_result_str(w).c_str(),
        get_result_str(w).c_str());
      fflush(stdout);
      break;
    }
    cur = 3 - cur;
  }
  return 0;
}

