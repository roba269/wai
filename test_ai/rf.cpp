#include <cstdio>
#include <unistd.h>

int main() {
  int pid = fork();
  if (pid == 0) {
    fprintf(stderr, "i am child\n");
    while (1);
  } else {
    fprintf(stderr, "i am parent\n");
    while (1);
  }
  return 0;
}

