#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <string>
using namespace std;

const int BUF_LEN = 128;
const int R = 8;
const int C = 8;

enum ChessType {
  B_KING = 1,
  B_QUEEN,
  B_ROOK,
  B_BISHOP,
  B_KNIGHT,
  B_PAWN,
  W_KING = 11,
  W_QUEEN,
  W_ROOK,
  W_BISHOP,
  W_KNIGHT,
  W_PAWN,
};

const int dir_knight[][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},
  {1,-2}, {1,2}, {2,-1}, {2,1}};

int bd[R][C];
int is_init[R][C];  // for castling && pawn's double move
int pass_x, pass_y; // for "in pass" pawn

int is_in_check(int);

inline bool in(int x, int y) {
  return x >= 0 && x < R && y >= 0 && y < C;
}

void init_board() {
  memset(bd, 0, sizeof(bd));
  bd[0][0] = bd[0][7] = B_ROOK;
  bd[0][1] = bd[0][6] = B_KNIGHT;
  bd[0][2] = bd[0][5] = B_BISHOP;
  bd[0][3] = B_QUEEN;
  bd[0][4] = B_KING;
  for (int i = 0 ; i < 8 ; ++i)
    bd[1][i] = B_PAWN;
  for (int i = 6 ; i < 8 ; ++i)
    for (int j = 0 ; j < C ; ++j)
      bd[i][j] = bd[7-i][j] + 10;
  for (int i = 0 ; i < R ; ++i)
    for (int j = 0 ; j < C ; ++j)
      is_init[i][j] = 1;
}

inline void move_chess(int cur, int x1, int y1, int x2, int y2, int real = 1) {
  if ((bd[x1][y1] == W_KING || bd[x1][y1] == B_KING) && 
      abs(y1 - y2) == 2) {
    // castling
    if (y2 < y1) {
      // O-O-O
      bd[x1][3] = (cur == 1 ? W_ROOK : B_ROOK);
      bd[x1][0] = 0;
      if (real) is_init[x1][0] = 0;
    } else {
      // O-O
      bd[x1][5] = (cur == 1 ? W_ROOK : B_ROOK);
      bd[x1][7] = 0;
      if (real) is_init[x1][7] = 0;
    }
  }
  if (real) {
    if ((bd[x1][y1] == W_PAWN || bd[x1][y1] == B_PAWN) &&
        abs(x1 - x2) == 2) {
      // record in pass pawn
      pass_x = (x1 + x2) / 2;
      pass_y = y1;
    } else {
      pass_x = pass_y = -1;
    }
  }
  if ((bd[x1][y1] == W_PAWN || bd[x1][y1] == B_PAWN) && y1 != y2) {
    if (bd[x2][y2] == 0) {
      // capture in pass pawn
      bd[x1][y2] = 0;
    }
  }
  bd[x2][y2] = bd[x1][y1];
  bd[x1][y1] = 0;
  if (real) is_init[x1][y1] = 0;
}

inline int has_chess(int cur, int x, int y) {
  if (cur == 1) return bd[x][y] >= W_KING && bd[x][y] <= W_PAWN;
  if (cur == 2) return bd[x][y] >= B_KING && bd[x][y] <= B_PAWN;
  return 0;
}

inline int check_empty_line(int x1, int y1, int x2, int y2) {
  if (x1 == x2) {
    for (int t = min(y1,y2) + 1 ; t <= max(y1,y2) - 1 ; ++t)
      if (bd[x1][t] != 0) return 0;
    return 1;
  } else if (y1 == y2) {
    for (int t = min(x1,x2) + 1 ; t <= max(x1,x2) - 1 ; ++t)
      if (bd[t][y1] != 0) return 0;
    return 1;
  }
  return 0;
}

inline int check_empty_diag(int x1, int y1, int x2, int y2) {
  if (x1 + y1 == x2 + y2) {
    for (int t = min(x1,x2) + 1 ; t <= max(x1,x2) - 1 ; ++t) {
      if (bd[t][x1 + y1 - t] != 0) return 0;
    }
    return 1;
  }
  if (x1 - y1 == x2 - y2) {
    for (int t = min(x1,x2) + 1 ; t <= max(x1,x2) - 1 ; ++t) {
      if (bd[t][t - x1 + y1] != 0) return 0;
    }
    return 1;
  }
  return 0;
}

inline int is_init_king(int cur, int x, int y) {
  int tmpx = (cur == 1 ? 7 : 0);
  if (x != tmpx || y != 4) return 0;
  return is_init[x][y];
}

inline int is_init_rook(int cur, int x, int y) {
  int tmpx = (cur == 1 ? 7 : 0);
  if (x != tmpx || (y != 0 && y != 7)) return 0;
  return is_init[x][y];
}

inline int is_init_pawn(int cur, int x, int y) {
  int tmpx = (cur == 1 ? 6 : 1);
  if (x != tmpx) return 0;
  return is_init[x][y];
}

int check_king(int cur, int x1, int y1, int x2, int y2, char *str) {
  if (is_init_king(cur, x1, y1) && x1 == x2 && abs(y1 - y2) == 2) {
    int rooky;
    if (y1 > y2) {
      // O-O-O
      rooky = 0;
    } else {
      // O-O
      rooky = 7;
    }
    if (is_init_rook(cur, x1, rooky) == 0) {
      strcpy(str, "Wrong_castling:_Rook_is_not_at_init_pos.");
      return 0;
    }
    for (int ty = min(y1, rooky) + 1 ; ty < max(y1, rooky) ; ++ty)
    {
      if (bd[x1][ty] != 0) {
        strcpy(str, "Wrong_castling:_Blocked_by_pieces.");
        return 0;
      }
    }
    int tmp_bd[R][C];
    for (int ty = min(y1,y2) ; ty <= max(y1,y2) ; ++ty) {
      memcpy(tmp_bd, bd, sizeof(bd));
      bd[x1][y1] = 0;
      bd[x1][ty] = (cur == 1 ? W_KING : B_KING);
      if (is_in_check(cur)) {
        memcpy(bd, tmp_bd, sizeof(bd));
        strcpy(str, "Wrong_castling:_position_in_attack.");
        return 0;
      }
      memcpy(bd, tmp_bd, sizeof(bd));
    }
    return 1;
  }
  if (abs(x1-x2) > 1 || abs(y1-y2) > 1) {
    strcpy(str, "King_moves_too_far.");
    return 0;
  }
  return 1;
}

int check_queen(int cur, int x1, int y1, int x2, int y2, char *str) {
  if (check_empty_line(x1, y1, x2, y2) == 0 &&
      check_empty_diag(x1, y1, x2, y2) == 0) {
    strcpy(str, "Queen_invalid_move.");
    return 0;
  }
  return 1;
}

int check_rook(int cur, int x1, int y1, int x2, int y2, char *str) {
  if (check_empty_line(x1, y1, x2, y2) == 0) {
    strcpy(str, "Rook_invalid_move.");
    return 0;
  }
  return 1;
}

int check_bishop(int cur, int x1, int y1, int x2, int y2, char *str) {
  if (check_empty_diag(x1, y1, x2, y2) == 0) {
    strcpy(str, "Bishop_invalid_move.");
    return 0;
  }
  return 1;
}

int check_knight(int cur, int x1, int y1, int x2, int y2, char *str) {
  for (int d = 0 ; d < 8 ; ++d) {
    int tx = x1 + dir_knight[d][0];
    int ty = y1 + dir_knight[d][1];
    if (tx == x2 && ty == y2) {
      return 1;
    }
  }
  strcpy(str, "Knight_invalid_move.");
  return 0;
}

int check_pawn(int cur, int x1, int y1, int x2, int y2, char *str)
{
  // only forward
  if ((cur == 1 && x2 >= x1) || (cur == 2 && x2 <= x1)) {
    strcpy(str, "Pawn_must_move_forward.");
    return 0;
  }
  if (y1 == y2) {
    if (abs(x1-x2) > 2) {
      strcpy(str, "Pawn_move_too_far.");
      return 0;
    }
    if (abs(x1-x2) == 2) {
      if (!is_init_pawn(cur, x1, y1)) {
        strcpy(str, "Pawn_not_at_inital_pos_cannot_move_two_squares.");
        return 0;
      }
      if (bd[(x1+x2)/2][y2] != 0 || bd[x2][y2] != 0) {
        strcpy(str, "Pawn_moving_is_blocked.");
        return 0;
      }
    }
    if (bd[x2][y2] != 0) {
      strcpy(str, "Pawn_moving_is_blocked.");
      return 0;
    }
  } else {
    if (abs(y1-y2) > 1 || abs(x1-x2) > 1) {
      strcpy(str, "Pawn_move_diagonally_too_far.");
      return 0;
    }
    if ((bd[x2][y2] == 0 && (x2 != pass_x || y2 != pass_y))
        || has_chess(cur, x2, y2)) {
      strcpy(str, "Pawn_without_capture_cannot_move_diagonally.");
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
    case W_KING:
      return check_king(cur, x1, y1, x2, y2, str);
    case W_QUEEN:
      return check_queen(cur, x1, y1, x2, y2, str);
    case W_ROOK:
      return check_rook(cur, x1, y1, x2, y2, str);
    case W_BISHOP:
      return check_bishop(cur, x1, y1, x2, y2, str);
    case W_KNIGHT:
      return check_knight(cur, x1, y1, x2, y2, str);
    case W_PAWN:
      return check_pawn(cur, x1, y1, x2, y2, str);
  }
  strcpy(str, "Invalid_chess_type.");
  return 0;
}

int is_in_check(int cur) {
  int kx, ky;
  for (kx = 0 ; kx < R ; ++kx)
    for (ky = 0 ; ky < C ; ++ky)
      if ((cur == 1 && bd[kx][ky] == W_KING) ||
          (cur == 2 && bd[kx][ky] == B_KING)) goto out;
out:
  char str[BUF_LEN];
  for (int x = 0 ; x < R ; ++x)
    for (int y = 0 ; y < C ; ++y)
      if (has_chess(3-cur, x, y) &&
          valid(3-cur, x, y, kx, ky, str)) return 1;
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

int is_checkmate(int cur) {
  int tmp_bd[R][C];
  char buf[BUF_LEN];
  if (is_in_check(cur) == 0) return 0;
  for (int x1 = 0 ; x1 < R ; ++x1)
    for (int y1 = 0 ; y1 < C ; ++y1) {
      if (!has_chess(cur, x1, y1))
        continue;
      for (int x2 = 0 ; x2 < R ; ++x2)
        for (int y2 = 0 ; y2 < C ; ++y2) {
          if (valid(cur, x1, y1, x2, y2, buf)) {
            memcpy(tmp_bd, bd, sizeof(bd));
            move_chess(cur, x1, y1, x2, y2, 0);
            if (!is_in_check(cur)) {
              memcpy(bd, tmp_bd, sizeof(bd));
              return 0;
            }
            memcpy(bd, tmp_bd, sizeof(bd));
          }
        }
    }
  return 1;
}

int is_stalemate(int cur) {
  int tmp_bd[R][C];
  char buf[BUF_LEN];
  if (is_in_check(cur)) return 0;
  for (int x1 = 0 ; x1 < R ; ++x1)
    for (int y1 = 0 ; y1 < C ; ++y1) {
      if (!has_chess(cur, x1, y1))
        continue;
      for (int x2 = 0 ; x2 < R ; ++x2)
        for (int y2 = 0 ; y2 < C ; ++y2) {
          if (valid(cur, x1, y1, x2, y2, buf)) {
            memcpy(tmp_bd, bd, sizeof(bd));
            move_chess(cur, x1, y1, x2, y2, 0);
            if (!is_in_check(cur)) {
              memcpy(bd, tmp_bd, sizeof(bd));
              return 0;
            }
            memcpy(bd, tmp_bd, sizeof(bd));
          }
        }
    }
  return 1;
}

int is_promoted(int cur, int *ox, int *oy) {
  for (int ty = 0 ; ty < C ; ++ty) {
    if (cur == 1 && bd[0][ty] == W_PAWN) {
      *ox = 0; *oy = ty; return 1;
    }
    if (cur == 2 && bd[7][ty] == B_PAWN) {
      *ox = 7; *oy = ty; return 1;
    }
  }
  return 0;
}

int pro_type_to_chess_type(int cur, int pro_type) {
  int res;
  switch (pro_type) {
    case 0: res = W_QUEEN; break;
    case 1: res = W_ROOK; break;
    case 2: res = W_BISHOP; break;
    case 3: res = W_KNIGHT; break;
    default: res = W_QUEEN; break;
  }
  if (cur == 2) res = res - 10;
  return res;
}

int get_winner() {
  int flg_b = 0, flg_w = 0;
  for (int i = 0 ; i < R ; i++)
      for (int j = 0 ; j < C ; j++)
        if (bd[i][j] == B_KING) flg_b = 1; 
        else if (bd[i][j] == W_KING) flg_w = 1;
  if (!flg_b) return 1;
  if (!flg_w) return 2;
  return 0;
}

inline string get_result_str(int side) {
  if (side == 0) return "Draw";
  if (side == 1) return "Player_1_win";
  return "Player_2_win";
}

void output_board() {
/*
  for (int i = 0 ; i < R ; ++i) {
    for (int j = 0 ; j < C ; ++j)
      fprintf(stderr, "%2d ", bd[i][j]);
    fprintf(stderr, "\n");
  }
*/
}

int main() {
  char tmp_buf[BUF_LEN + 10];
  char buf[BUF_LEN + 10];
  init_board();
  printf(">1: first\n");
  fflush(stdout);
  printf(">2: second\n");
  fflush(stdout);
  int cur = 1;
  pass_x = pass_y = -1;
  while (1) {
    output_board();
    if (is_checkmate(cur)) {
      printf("%d %s Checkmate.\n", 3-cur,
          get_result_str(3-cur).c_str());
      fflush(stdout);
      break;
    } else if (is_stalemate(cur)) {
      printf("0 %s Stalemate.\n", get_result_str(0).c_str());
      fflush(stdout);
      break;
    }
    printf("<%d\n", cur);
    fflush(stdout);
    int x1, y1, x2, y2;
    fgets(tmp_buf, BUF_LEN, stdin);
    sscanf(tmp_buf, "%d %d %d %d", &x1, &y1, &x2, &y2);
    if (!valid(cur, x1, y1, x2, y2, buf)) {
      printf("%d %s Invalid_move:_%s\n", 3-cur,
          get_result_str(3-cur).c_str(), buf);
      fflush(stdout);
      break;
    }
    // printf("+%d %d %d %d %d\n", cur, x1, y1, x2, y2);
    // fflush(stdout);
    move_chess(cur, x1, y1, x2, y2);
    printf(">%d: %d %d %d %d\n", 3-cur, x1, y1, x2, y2);
    fflush(stdout);
    int px, py;
    if (is_promoted(cur, &px, &py)) {
      printf("<%d\n", cur);
      fflush(stdout);
      fgets(tmp_buf, BUF_LEN, stdin);
      int pro_type;
      sscanf(tmp_buf, "%d", &pro_type);
      bd[px][py] = pro_type_to_chess_type(cur, pro_type);
      // printf("+%d\n", pro_type);
      // fflush(stdout);
      printf(">%d: %d\n", 3-cur, pro_type);
      fflush(stdout);
      printf("+%d %d %d %d %d %d\n", cur, x1, y1, x2, y2, pro_type);
      fflush(stdout);
    } else {
      printf("+%d %d %d %d %d\n", cur, x1, y1, x2, y2);
      fflush(stdout);
    }

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

