#include <cstdio>
#include <unistd.h>

int main() {
  FILE *fp = fopen("hello.txt","w");
  fclose(fp);
  return 0;
}

