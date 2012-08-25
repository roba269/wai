#include <cstdio>
#include <cstring>

int main() {
  char buf[16];
  scanf("%s", buf);
  if (strcmp(buf, "first") == 0) {
    printf("7 7\n");
  }
  while (1) {
    int x, y;
    scanf("%d %d", &x, &y);
    printf("%d %d\n", x+1, y+1);
  }
  return 0;
}

