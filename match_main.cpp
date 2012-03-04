#include <cstdio>
#include <string>
#include "simple_match.h"
#include "sandbox.h"
#include "common_define.h"

SimpleMatch sm;

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage %s: judge p1 p2\n", argv[0]);
        return 1;
    }
    sm.InitMatch(MATCH_RENJU, 1001, "test renju");
    Sandbox p1(argv[2]), p2(argv[3]), judge(argv[1]);
    sm.SetJudge(&judge);
    sm.AddPlayer(&p1);
    sm.AddPlayer(&p2);
    sm.Start();
    return 0;
}

