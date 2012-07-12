#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "hvc_match.h"
#include "sandbox.h"
#include "common_define.h"

HVCMatch hm;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage %s: judge computer\n", argv[0]);
        return 1;
    }
    hm.InitMatch(MATCH_RENJU, 1002, "test hvc match");
    Sandbox judge(argv[1]), comp(argv[2]);
    hm.SetJudge(&judge);
    hm.SetComputer(&comp, 0);
    hm.Start();
    return 0;
}
