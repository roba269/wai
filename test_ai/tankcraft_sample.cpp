#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cassert>
#include <unistd.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)>0?(x):(-(x)))
#define CLEAR(x,v) memset((x), (v), sizeof((x)))

const int MAP_HEIGHT = 21;
const int MAP_WIDTH = 21;
const int MAX_MINE_CNT = 100;
const int INF = 100000000;
const int MAXQ = (MAP_HEIGHT+4)*(MAP_WIDTH+4);
const int dir[][2] = {{-1,0},{1,0},{0,-1},{0,1}};
enum CmdType {
  GOUP = 0,
  GODOWN,
  GOLEFT,
  GORIGHT,
  STOP,
  FIRE,
  INVALID,
};
const char *cmd2str[] = {"GOUP", "GODOWN", "GOLEFT", "GORIGHT", "STOP"};
const CmdType dir2order[] = {GOUP, GODOWN, GOLEFT, GORIGHT};

int pid;
int mine_cnt;

enum TankType {
  STRIKER = 0,
  SNIPER,
  PIONEER,
};

enum MapCellType {
  EMPTY = 0,
  STONE,
  BRICK,
  BROKEN_BRICK,
} mp[MAP_HEIGHT+2][MAP_WIDTH+2];

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
};

struct Mine {
  int x, y;
  int owner; // 0: red 1: blue -1: none
} mine[MAX_MINE_CNT];

static int q[MAXQ][2];
static int step[MAP_HEIGHT+2][MAP_WIDTH+2];
static int prev[MAP_HEIGHT+2][MAP_WIDTH+2];
static int inq[MAP_HEIGHT+2][MAP_WIDTH+2];

inline int anti_dir(int d) {
  if (d == -1) return d;
  return (d<2)?(1-d):(5-d);
}

inline Cmd gen_order(int x, int y, int d) {
  int tx = x + dir[d][0], ty = y + dir[d][1];
  Cmd order;
  if (mp[tx][ty] != EMPTY) { 
    order.type = FIRE;
    order.x = tx; order.y = ty;
  } else order.type = dir2order[d];
  return order;
}

void find_path(int me, int tank_id, int mine_id, Cmd &order) {
  int qh, qt, d, tx, ty, sx, sy;
  order.type = STOP;
  sx = q[0][0] = mine[mine_id].x;
  sy = q[0][1] = mine[mine_id].y;
  if (me == mine[mine_id].owner)
    return;

  memset(step, 0x3f, sizeof(step));
  memset(inq, 0, sizeof(inq));
  
  step[sx][sy] = 0;
  inq[sx][sy] = 1;
  for (qh = 0, qt = 1; qh != qt ; ) {
    for (d = 0 ; d < 4 ; d++) {
      tx = q[qh][0] + dir[d][0];
      ty = q[qh][1] + dir[d][1];
      if (mp[tx][ty] == STONE) continue;
      int tcost = 0;
      if (mp[tx][ty] == EMPTY) tcost = 1;
      else if (mp[tx][ty] == BRICK) {
        tcost = 3;
      } else if (mp[tx][ty] == BROKEN_BRICK) tcost = 2;
      
      if (step[tx][ty] > step[q[qh][0]][q[qh][1]] + tcost) {
        step[tx][ty] = step[q[qh][0]][q[qh][1]] + tcost;
        prev[tx][ty] = d;
        if (!inq[tx][ty]) {
          inq[tx][ty] = 1;
          q[qt][0] = tx; q[qt][1] = ty;
          if (++qt == MAXQ) qt = 0;
        }
      }
    }
    inq[q[qh][0]][q[qh][1]] = 0;
    if (++qh == MAXQ) qh = 0;
  }
  int fx = tank[me][tank_id].x;
  int fy = tank[me][tank_id].y;
  d = anti_dir(prev[fx][fy]);
  order = gen_order(fx, fy, d);
}

int check_nearby(int me, int tank_id, Cmd &order) {
  int x = tank[me][tank_id].x;
  int y = tank[me][tank_id].y;
  for (int i = 0 ; i < 5 ; ++i) {
    if (tank[1-me][i].x > 0) {
      int dis = ABS(tank[1-me][i].x - x) + ABS(tank[1-me][i].y - y);
      if (dis <= tank[me][tank_id].range) {
        order.type = FIRE;
        order.x = tank[1-me][i].x;
        order.y = tank[1-me][i].y;
        return 1;
      }
    }
  }
  return 0;
}

bool is_target[MAX_MINE_CNT];

Cmd make_order(int me, int tank_id)
{
  Cmd cmd;
  if (tank[me][tank_id].x < 0) {
    cmd.type = STOP;
    return cmd;
  }
  int tmp = check_nearby(me, tank_id, cmd);
  if (tmp != 0) return cmd;

  // no enemy nearby, find any un-targeted mine
  for (int mine_id = 0 ; mine_id < mine_cnt ; ++mine_id) {
    if (mine[mine_id].owner == me) continue;
    if (is_target[mine_id]) continue;
    is_target[mine_id] = 1;
    find_path(me, tank_id, mine_id, cmd);
    return cmd;
  }
  cmd.type = STOP;
  return cmd;
}

void parse_map_buf(const char *buf) {
  int mine_idx = 0;
  int p = 0;
  for (int i = 0 ; i < MAP_HEIGHT + 2 ; ++i) {
    for (int j = 0 ; j < MAP_WIDTH + 2 ; ++j) {
      // parse the map buf
      switch (buf[p]) {
        case 'S':
          mp[i][j] = STONE;
          break;
        case 'B':
          mp[i][j] = BRICK;
          break;
        case 'b':
          mp[i][j] = BROKEN_BRICK;
          break;
        default:
          mp[i][j] = EMPTY;
          break;
      }
      if (isdigit(buf[p])) {
        int idx = buf[p] - '0';
        tank[idx/5][idx%5].born_x = i;
        tank[idx/5][idx%5].born_y = j;
      } else if (buf[p] == 'G') {
        mine[mine_idx].x = i;
        mine[mine_idx].y = j;
        mine[mine_idx].owner = -1;
        ++mine_idx;
      } else if (buf[p] == 'H') {
        mine[mine_idx].x = i;
        mine[mine_idx].y = j;
        mine[mine_idx].owner = 0;
        ++mine_idx;
      } else if (buf[p] == 'L') {
        mine[mine_idx].x = i;
        mine[mine_idx].y = j;
        mine[mine_idx].owner = 1;
        ++mine_idx;
      }
      ++p;
    }
  }
  mine_cnt = mine_idx;
  assert(p == (MAP_WIDTH+2) * (MAP_HEIGHT+2));
}

char buf[1024];
int score[2], kill[2];

int main() {
  pid = getpid();
  int me;
  scanf("%s", buf);
  if (strcmp(buf, "red") == 0) me = 0;
  else me = 1;
  scanf("%s", buf);
  parse_map_buf(buf);
  printf("%d %d %d %d %d\n", STRIKER, STRIKER,
      SNIPER, SNIPER, PIONEER);
  fflush(stdout);
  while (1) {
    int rnd;
    scanf("%d %s %d %d %d %d", &rnd, buf, &score[0], &score[1],
        &kill[0], &kill[1]);
    for (int i = 0 ; i < 2 ; i++)
      for (int j = 0 ; j < 5 ; j++) {
        scanf("%d %d %d %d %d %d", &tank[i][j].x, &tank[i][j].y,
            &tank[i][j].type, &tank[i][j].hp,
            &tank[i][j].revive, &tank[i][j].noharm);
        tank[i][j].range = tank_type_range[tank[i][j].type];
      }
    parse_map_buf(buf);
    memset(is_target, 0, sizeof(is_target));
    for (int j = 0 ; j < 5 ; ++j) {
      Cmd cmd = make_order(me, j);
      if (cmd.type == FIRE) {
        printf("%s %d %d\n", "FIRE", cmd.x, cmd.y);
        fflush(stdout);
      } else {
        printf("%s\n", cmd2str[cmd.type]);
        fflush(stdout);
      }
    }
  }
  return 0;
}

