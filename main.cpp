#include "player.h"
#include "match_renju.h"

MatchRenju g_match;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s player1.exe player2.exe", argv[0]);
        return 1;
    }
    Player p1, p2;
    p1.SetName("roba");
    p1.SetType(PLAYER_COMPUTER);
    p2.SetName("abor");
    p2.SetType(PLAYER_HUMAN);
    p1.LoadAI(argv[1]);
    p2.LoadAI(argv[2]);
    g_match.Init("Test Renju Match");
    g_match.AddPlayer(&p1);
    g_match.AddPlayer(&p2);
    g_match.Start();
    printf("%d\n", g_match.CheckWinner());
    return 0;
}

