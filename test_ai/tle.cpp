#include <cstdio>

int main() {
    int tot = 0, i, j;
    for (i = 0 ; i < 100000 ; i++)
        for (j = 0 ; j < 100000 ; j++)
            tot += i * j;
    printf("%d\n", tot);
    return 0;
}

