#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <libgen.h>
#include <cassert>
using namespace std;

const int WIDTH = 21;
const int HEIGHT = 21;
const int INF = 1000000;
const int MAX_MINE_CNT = 100;
const int NO_HARM_ROUND = 2;
const int REVIVE_ROUND = 2;
const int MAX_SCORE = 1000;

int mp[HEIGHT+2][WIDTH+2];
char map_buf[1024];
int mine_cnt;

const int dir[][2] = {{-1,0},{1,0},{0,-1},{0,1},{0,0}};

enum CmdType {
  GOUP = 0,
  GODOWN,
  GOLEFT,
  GORIGHT,
  STOP,
  FIRE,
  INVALID,
};

enum TankType {
  STRIKER = 0,
  PIONEER,
  SNIPER,
};

enum MapCellType {
  EMPTY = 0,
  STONE,
  BRICK,
  BROKEN_BRICK,
};

const int tank_type_hp[] = {3, 9, 2};
const int tank_type_range[] = {3, 1, 5};
const int tank_type_attack[] = {2, 1, 1};

struct Tank {
  int x, y, type;
  int attack, range, hp;
  int noharm, revive;
  int born_x, born_y;
} tank[2][5];

struct Cmd {
  CmdType type;
  int x, y;
} cmd[2][5];

struct Mine {
  int x, y;
  int owner; // 0: red 1: blue -1: none
} mine[MAX_MINE_CNT];

int score[2];
int own_mine[2];
int kill[2];
int rnd;

typedef pair<int,int> PII;
vector<PII> hit_at_middle;
vector<PII> hit_queue[HEIGHT+2][WIDTH+2];

inline int in_board(int x, int y) {
  return x >= 1 && x <= HEIGHT && y >= 1 && y <= WIDTH;
}

inline int is_oppo_dir(CmdType d1, CmdType d2) {
  return (d1 == GOUP && d2 == GODOWN) ||
    (d1 == GODOWN && d2 == GOUP) ||
    (d1 == GOLEFT && d2 == GORIGHT) ||
    (d1 == GORIGHT && d2 == GOLEFT);
}

inline void kill_tank(int i, int j) {
  tank[i][j].hp = 0;
  tank[i][j].x = -1;
  tank[i][j].y = -1;
  tank[i][j].revive = REVIVE_ROUND;
  ++kill[i];
}

inline int is_killed(int i, int j) {
  return tank[i][j].hp <= 0;
}

void init() {
  memset(tank, 0, sizeof(tank));
  memset(score, 0, sizeof(score));
  memset(mp, 0, sizeof(mp));
  memset(own_mine, 0, sizeof(own_mine));
  memset(kill, 0, sizeof(kill));
}

void process_fire_cmd() {
  for (int i = 0 ; i < 2 ; ++i)
    for (int j = 0 ; j < 5 ; ++j) {
      if (is_killed(i,j)) continue;
      if (cmd[i][j].type == FIRE) {
        int fx = cmd[i][j].x;
        int fy = cmd[i][j].y;
        if (!in_board(fx, fy)) continue;
        int dis = abs(fx - tank[i][j].x) + abs(fy - tank[i][j].y);
        if (dis > tank[i][j].range) continue;
        for (int ti = 0 ; ti < 2 ; ++ti)
          for (int tj = 0 ; tj < 5 ; ++tj) {
            if (is_killed(ti, tj)) continue;
            if (tank[ti][tj].x == fx && tank[ti][tj].y == fy) {
              if (tank[ti][tj].noharm == 0) {
                tank[ti][tj].hp -= tank[i][j].attack;
                if (tank[ti][tj].hp <= 0) {
                  kill_tank(ti, tj);
                }
              }
            }
          }
        if (mp[fx][fy] == BRICK) {
          if (tank[i][j].attack == 1) {
            mp[fx][fy] = BROKEN_BRICK;
          } else mp[fx][fy] = EMPTY;
        } else if (mp[fx][fy] == BROKEN_BRICK) {
          mp[fx][fy] = EMPTY;
        }
      }
  }
}

void process_move_cmd() {
  for (int x = 0 ; x < HEIGHT + 2 ; ++x)
    for (int y = 0 ; y < WIDTH + 2 ; ++y) {
      hit_queue[x][y].clear();
    }
  hit_at_middle.clear();
  for (int i = 0 ; i < 2 ; ++i)
    for (int j = 0 ; j < 5 ; ++j) {
      if (is_killed(i, j)) continue;
      if (cmd[i][j].type != FIRE) {
        int tx = tank[i][j].x + dir[cmd[i][j].type][0];
        int ty = tank[i][j].y + dir[cmd[i][j].type][1];
        int stay = 0;
        for (int prej = 0 ; prej < j ; ++prej) {
          if (tank[i][prej].x == tx && tank[i][prej].y == ty) {
            // target taken by friend tank, stay still
            stay = 1;
          }
        }
        if (!stay) {
          tank[i][j].x = tx;
          tank[i][j].y = ty;
          hit_queue[tx][ty].push_back(make_pair(i, j));
        }
        if (i == 0) {
          // check hit at middle
          for (int tmpj = 0 ; tmpj < 5 ; ++tmpj) {
            if (tank[1-i][tmpj].x == tx && tank[1-i][tmpj].y == ty &&
                is_oppo_dir(cmd[i][j].type, cmd[1-i][tmpj].type)) {
              hit_at_middle.push_back(make_pair(j, tmpj));
              break;
            }
          }
        }
      }
    }
}

void process_reborn() {
  for (int i = 0 ; i < 2 ; ++i)
    for (int j = 0 ; j < 5 ; ++j) {
      if (tank[i][j].revive == -1) continue;
      if (tank[i][j].revive == 0) {
        fprintf(stderr, "tank[%d][%d] revive\n", i, j);
        tank[i][j].revive = -1;
        tank[i][j].x = tank[i][j].born_x;
        tank[i][j].y = tank[i][j].born_y;
        tank[i][j].hp = tank_type_hp[tank[i][j].type];
        tank[i][j].noharm = NO_HARM_ROUND;
        hit_queue[tank[i][j].x][tank[i][j].y].push_back(make_pair(i, j));
      }
    }
}

inline int get_real_hp(int i, int j) {
  if (tank[i][j].noharm) return INF;
  return tank[i][j].hp;
}

void process_tank_hit() {
  for (int i = 0 ; i < 2 ; ++i)
    for (int j = 0 ; j < 5 ; ++j)
      if (!is_killed(i,j) && !in_board(tank[i][j].x, tank[i][j].y)) {
        kill_tank(i, j);
      }
  // process hit-at-middle
  for (int tt = 0 ; tt < hit_at_middle.size() ; ++tt) {
    int j1 = hit_at_middle[tt].first;
    int j2 = hit_at_middle[tt].second;
    // int dec = min(tank[0][j1].hp, tank[1][j2].hp);
    int dec = min(get_real_hp(0, j1), get_real_hp(1, j2));
    if (tank[0][j1].noharm == 0) {
      tank[0][j1].hp -= dec;
      if (tank[0][j1].hp <= 0) {
        kill_tank(0, j1);
      }
    }
    if (tank[1][j2].noharm == 0) {
      tank[1][j2].hp -= dec;
      if (tank[1][j2].hp <= 0) {
        kill_tank(1, j2);
      }
    }
  }
  for (int x = 0 ; x < HEIGHT + 2 ; ++x)
    for (int y = 0 ; y < WIDTH + 2 ; ++y) {
      if (mp[x][y] == STONE) {
        for (int tt = 0 ; tt < hit_queue[x][y].size() ; ++tt) {
          kill_tank(hit_queue[x][y][tt].first,
              hit_queue[x][y][tt].second);
        }
        continue;
      } else if (mp[x][y] == BRICK) {
        // at most two tank of diff sides hit brick
        if (hit_queue[x][y].size() >= 2) {
          mp[x][y] = EMPTY;
          for (int tt = 0 ; tt < 2 ; ++tt) {
            int i = hit_queue[x][y][tt].first;
            int j = hit_queue[x][y][tt].second;
            assert(!is_killed(i,j));
            if (tank[i][j].noharm == 0 && --tank[i][j].hp <= 0) {
              kill_tank(i, j);
            }
          }
        } else if (hit_queue[x][y].size() == 1) {
          int i = hit_queue[x][y][0].first;
          int j = hit_queue[x][y][0].second;
          assert(!is_killed(i, j));
          int dec = min(get_real_hp(i,j), 2);
          if (tank[i][j].noharm == 0) {
            tank[i][j].hp -= dec;
            if (tank[i][j].hp <= 0) {
              kill_tank(i,j);
            }
          }
          if (dec == 1) {
            mp[x][y] = BROKEN_BRICK;
          } else if (dec >= 2) {
            mp[x][y] = EMPTY;
          }
        }
      } else if (mp[x][y] == BROKEN_BRICK) {
        if (hit_queue[x][y].size() >= 1) {
          // always let red rank hit the brick, is it 
          int i = hit_queue[x][y][0].first;
          int j = hit_queue[x][y][0].second;
          mp[x][y] = EMPTY;
          assert(!is_killed(i,j));
          if (tank[i][j].noharm == 0 && --tank[i][j].hp <= 0) {
            kill_tank(i, j);
          }
        }
      }
      // empty space or fall through from BRICK space
      if (hit_queue[x][y].size() > 1) {
        int real_cnt = 0, dec = INF;
        for (int idx = 0 ; idx < hit_queue[x][y].size() ; ++idx) {
          int i = hit_queue[x][y][idx].first;
          int j = hit_queue[x][y][idx].second;
          if (!is_killed(i, j)) {
            ++real_cnt;
            dec = min(dec, get_real_hp(i,j));
          }
        }
        if (real_cnt > 1) {
          for (int idx = 0 ; idx < hit_queue[x][y].size() ; ++idx) {
            int i = hit_queue[x][y][idx].first;
            int j = hit_queue[x][y][idx].second;
            if (!is_killed(i, j) && tank[i][j].noharm == 0) {
              tank[i][j].hp -= dec;
              if (tank[i][j].hp <= 0) kill_tank(i, j);
            }
          }
        }
      }
    }
}

void process_mine_owner() {
  for (int idx_mine = 0 ; idx_mine < mine_cnt ; ++idx_mine) {
    for (int i = 0 ; i < 2 ; i++)
      for (int j = 0 ; j < 5 ; j++) {
        if (is_killed(i,j)) continue;
        if (tank[i][j].x == mine[idx_mine].x && 
            tank[i][j].y == mine[idx_mine].y)
        {
          if (mine[idx_mine].owner == 1-i) {
            --own_mine[1-i];
          }
          if (mine[idx_mine].owner != i) {
            mine[idx_mine].owner = i;
            ++own_mine[i];
          }
          goto out;
        }
      }
out:
    ;
  }
}

void update_score() {
  score[0] += own_mine[0];
  score[1] += own_mine[1];
  for (int i = 0 ; i < 2 ; ++i)
    for (int j = 0 ; j < 5 ; ++j) {
      if (tank[i][j].revive > 0)
        --tank[i][j].revive;
      if (tank[i][j].noharm > 0)
        if (--tank[i][j].noharm == 0)
          tank[i][j].hp = tank_type_hp[tank[i][j].type];
    }
}

int is_visible(int mi, int mj) {
  if (is_killed(mi, mj)) return 0;
  int mx = tank[mi][mj].x, my = tank[mi][mj].y;
  for (int idx = 0 ; idx < mine_cnt ; ++idx) {
    int dis = abs(mx - mine[idx].x) + abs(my - mine[idx].y);
    if (dis <= 2) return 1;
  }
  for (int ej = 0 ; ej < 5 ; ++ej) {
    int dis = abs(mx - tank[1-mi][ej].x) + abs(my - tank[1-mi][ej].y);
    if (dis <= 4) return 1;
  }
  return 0;
}

void load_map(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open %s\n", filename);
    exit(1);
  }
  char buf[64];
  int idx_mine = 0;
  for (int i = 0 ; i < HEIGHT + 2 ; ++i) {
    fscanf(fp, "%s", buf);
    for (int j = 0 ; j < WIDTH + 2 ; ++j) {
      int x = i, y = j;
      switch (buf[j]) {
        case 'S':
          mp[x][y] = STONE;
          break;
        case 'B':
          mp[x][y] = BRICK;
          break;
        case 'b':
          mp[x][y] = BROKEN_BRICK;
          break;
        default:
          mp[x][y] = EMPTY;
          if (isdigit(buf[j])) {
            int idx = buf[j] - '0';
            tank[idx/5][idx%5].born_x = x;
            tank[idx/5][idx%5].born_y = y;
          } else if (buf[j] == 'G') {
            mine[idx_mine].x = x;
            mine[idx_mine].y = y;
            mine[idx_mine].owner = -1;
            ++idx_mine;
          } else if (buf[j] == 'H') {
            mine[idx_mine].x = x;
            mine[idx_mine].y = y;
            mine[idx_mine].owner = 0;
            ++own_mine[0];
            ++idx_mine;
          } else if (buf[j] == 'L') {
            mine[idx_mine].x = x;
            mine[idx_mine].y = y;
            mine[idx_mine].owner = 1;
            ++own_mine[1];
            ++idx_mine;
          }
          break;
      }
    }
  }
  mine_cnt = idx_mine;
  fclose(fp);
}

void gen_map(char *buf) {
  int p = 0;
  for (int x = 0 ; x < HEIGHT + 2 ; ++x)
    for (int y = 0 ; y < WIDTH + 2 ; ++y) {
      if (mp[x][y] == STONE) buf[p++] = 'S';
      else if (mp[x][y] == BRICK) buf[p++] = 'B';
      else if (mp[x][y] == BROKEN_BRICK) buf[p++] = 'b';
      else {
        int flg = 0;
        for (int i = 0 ; i < 2 ; ++i)
          for (int j = 0 ; j < 5 ; ++j)
            if (tank[i][j].born_x == x &&
                tank[i][j].born_y == y) {
              buf[p++] = (i*5+j) + '0';
              flg = 1;
            }
        for (int i = 0 ; i < mine_cnt ; ++i)
          if (mine[i].x == x && mine[i].y == y) {
            flg = 1;
            if (mine[i].owner == -1)
              buf[p++] = 'G';
            else if (mine[i].owner == 0)
              buf[p++] = 'H';
            else
              buf[p++] = 'L';
          }
        if (!flg) buf[p++] = 'P';
      }
    }
  buf[p] = 0;
  assert(p == (HEIGHT+2)*(WIDTH+2));
}

void gen_info_for_client() {
  gen_map(map_buf);
  // send info to client view
  // round map score1 score2 hit1 hit2 [x y type hp revive noharm cmd cmd_x cmd_y]*10
  ostringstream oss;
  oss << rnd << " " << map_buf << " "
    << score[0] << " " << score[1] << " "
    << kill[0] << " " << kill[1];
  for (int fi = 0 ; fi < 2 ; ++fi)
    for (int fj = 0 ; fj < 5 ; ++fj) {
      oss << " " << tank[fi][fj].x
        << " " << tank[fi][fj].y
        << " " << (int)(tank[fi][fj].type)
        << " " << tank[fi][fj].hp
        << " " << tank[fi][fj].revive
        << " " << tank[fi][fj].noharm
        << " " << cmd[fi][fj].type
        << " " << cmd[fi][fj].x
        << " " << cmd[fi][fj].y;
    }
  printf("+%s\n", oss.str().c_str());
  fflush(stdout);
}

int main() {
  init();
  char tmp_path[128], full_path[128];
  int path_len = readlink("/proc/self/exe", tmp_path, BUFSIZ);
  tmp_path[path_len] = 0;
  strcpy(full_path, dirname(tmp_path));
  strcat(full_path, "/tankcraft_map/Arena.tankmap");
  // printf("full_path:%s\n", full_path);
  load_map(full_path);
  printf(">1: red\n");
  fflush(stdout);
  printf(">2: blue\n");
  fflush(stdout);
  gen_map(map_buf);
  printf(">1: %s\n", map_buf);
  fflush(stdout);
  printf(">2: %s\n", map_buf);
  fflush(stdout);
  for (int i = 0 ; i < 2 ; ++i) {
    printf("<%d\n", i+1);
    fflush(stdout);
    for (int j = 0 ; j < 5 ; j++) {
      scanf("%d", &tank[i][j].type);
      if (tank[i][j].type < 0 || tank[i][j].type > 2) {
        printf("%d WrongTankType\n", (1-i)+1);
        fflush(stdout);
        return 0;
      }
      tank[i][j].hp = tank_type_hp[tank[i][j].type];
      tank[i][j].range = tank_type_range[tank[i][j].type];
      tank[i][j].attack = tank_type_attack[tank[i][j].type];
      tank[i][j].noharm = 2;
      tank[i][j].revive = -1;
      tank[i][j].x = tank[i][j].born_x;
      tank[i][j].y = tank[i][j].born_y;
    }
  }
  for (rnd = 1 ; ; rnd++) {
    for (int i = 0 ; i < 2 ; ++i) {
      // send info to AI
      // round map score1 score2 hit1 hit2 [x y type hp revive noharm]*10
      ostringstream oss;
      gen_map(map_buf);
      oss << rnd << " " << map_buf << " "
        << score[0] << " " << score[1] << " "
        << kill[0] << " " << kill[1];
      for (int fi = 0 ; fi < 2 ; ++fi)
        for (int fj = 0 ; fj < 5 ; ++fj) {
          if (fi == i) { // own side, full info
            oss << " " << tank[fi][fj].x
              << " " << tank[fi][fj].y
              << " " << (int)(tank[fi][fj].type)
              << " " << tank[fi][fj].hp
              << " " << tank[fi][fj].revive
              << " " << tank[fi][fj].noharm;
          } else {  // enemy side
            int tmpx, tmpy;
            if (is_visible(fi, fj)) {
              tmpx = tank[fi][fj].x;
              tmpy = tank[fi][fj].y;
            } else tmpx = tmpy = -1;
            oss << " " << tmpx << " " << tmpy
              << " " << (int)(tank[fi][fj].type)
              << " " << tank[fi][fj].hp
              << " " << tank[fi][fj].revive
              << " " << tank[fi][fj].noharm;
          }
        }
      printf(">%d: %s\n", i+1, oss.str().c_str());
      fflush(stdout);
    }
    for (int i = 0 ; i < 2 ; ++i) {
      for (int j = 0 ; j < 5 ; ++j) {
        printf("<%d\n", i+1);
        fflush(stdout);
        char cmd_buf[16];
        scanf("%10s", cmd_buf);
        if (strcmp(cmd_buf, "GOUP") == 0) {
          cmd[i][j].type = GOUP;
        } else if (strcmp(cmd_buf, "GOLEFT") == 0) {
          cmd[i][j].type = GOLEFT;
        } else if (strcmp(cmd_buf, "GORIGHT") == 0) {
          cmd[i][j].type = GORIGHT;
        } else if (strcmp(cmd_buf, "GODOWN") == 0) {
          cmd[i][j].type = GODOWN;
        } else if (strcmp(cmd_buf, "STOP") == 0) {
          cmd[i][j].type = STOP;
        } else if (strcmp(cmd_buf, "FIRE") == 0) {
          cmd[i][j].type = FIRE;
          scanf("%d %d", &cmd[i][j].x, &cmd[i][j].y);
        } else {
          cmd[i][j].type = INVALID;
        }
      }
    }
    gen_info_for_client();
    process_fire_cmd();
    process_move_cmd();
    process_reborn();
    process_tank_hit();
    process_mine_owner();
    update_score();
    if (score[0] >= MAX_SCORE || score[1] >= MAX_SCORE)
      break;
  }
  char out_buf[128];
  sprintf(out_buf, "RedScore[%d]:BludScore[%d],RedKill[%d]:BlueKill[%d]", score[0], score[1], kill[0], kill[1]);
  if (score[0] == score[1]) {
    if (kill[0] == kill[1])
      printf("0 %s\n", out_buf);
    else if (kill[0] > kill[1])
      printf("1 %s\n", out_buf);
    else 
      printf("2 %s\n", out_buf);
  } else if (score[0] > score[1]) {
    printf("1 %s\n", out_buf);
  } else
    printf("2 %s\n", out_buf);
  fflush(stdout);
  return 0;
}

