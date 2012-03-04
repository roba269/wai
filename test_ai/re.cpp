#include <cstdio>

int a[10];

int main() {
    a[120000] = 1;
    ++a[120000];
    printf("%d\n",a[120000]*a[120001]);
    return 0;
}
