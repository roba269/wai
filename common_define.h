#ifndef COMMON_DEFINE_H
#define COMMON_DEFINE_H

#include <cstring>

enum ExitFlagType {
    EXIT_NONE,
    EXIT_NORMAL,
    EXIT_RF,
    EXIT_TLE,
    EXIT_MLE,
    EXIT_RE
};

enum MatchType {
    MATCH_RENJU,
    MATCH_OTHELLO,
    MAX_GAME_TYPE
};

inline int str2type(const char *str) {
    if (strcmp(str, "RENJU") == 0) return 0;
    if (strcmp(str, "OTHELLO") == 0) return 1;
    return -1;
}

#define RECORD_PREFIX "/home/roba/wai/record"
#define SRC_PREFIX "/home/roba/wai/submit"

#endif
