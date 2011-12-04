#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "player.h"
#include "match_renju.h"

MatchRenju g_match;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s player1.exe player2.exe\n", argv[0]);
        return 1;
    }
    PlayerComputer p1, p2;
    p1.SetName("roba");
    p1.SetType(PLAYER_COMPUTER);
    p2.SetName("abor");
    p2.SetType(PLAYER_HUMAN);
    if (p1.LoadAI(argv[1], 1001) < 0) {
        printf("Load AI %s error\n", argv[1]);
        return 1;
    }
    if (p2.LoadAI(argv[2], 1002) < 0) {
        printf("Load AI %s error\n", argv[2]);
        return 1;
    }
    g_match.InitMatch("Test Renju Match");
    g_match.AddPlayer(&p1);
    g_match.AddPlayer(&p2);
    g_match.Start();
    printf("The winner is: %d\n", g_match.CheckWinner());
    g_match.OutputStat();
    return 0;
}

