#include <cstdio>
#include <unistd.h>
#include <cstring>

int main() {
    int n, m;
    char buf[128];
    while (scanf("%d %d",&n,&m)) {
        // printf("%d %d\n",n+1,m+1);
        sprintf(buf, "%d %d\n", n+1, m+1);
        write(1, buf, strlen(buf));
        // fflush(stdout);
    }
    return 0;
}

