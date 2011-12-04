#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;

#define MYSELF(flg) 
#define ENEMY(flg)

char buf[16];
int flg;
int bd[16][16];

void pick_empty(int &x, int &y) {
    for (x = 0 ; x < 16 ; ++x)
        for (y = 0 ; y < 16 ; ++y)
            if (bd[x][y] == 0) return ;
}

int main() {
    fprintf(stderr, "hello, i am %d\n", getpid());
    scanf("%s", buf);
    fprintf(stderr, "%d: I recv %s\n", getpid(), buf);
    if (strcmp(buf, "First") == 0) {
        flg = 1;
    } else {
        flg = 2;
    }
    memset(bd, 0, sizeof(bd));
    if (flg == 2) {
        int x, y;
        scanf("%d %d", &x, &y);
        bd[x][y] = 3 - flg;
    }
    while (1) {
        int x, y;
        pick_empty(x, y);
        printf("%d %d\n", x, y);
        fflush(stdout);
        bd[x][y] = flg;
        scanf("%d %d", &x, &y);
        // fprintf(stderr, "%d: I recv [%d %d]\n", getpid(), x, y);
        bd[x][y] = 3 - flg;
    }
    return 0;
}

