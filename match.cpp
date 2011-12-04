#include <cassert>
#include <cstring>
#include <cstdio>
#include <sys/select.h>
#include "match.h"
#include "player.h"

int Match::SendMsg(int player_idx, const char *msg)
{
    assert(player_idx >= 0 && player_idx < m_players.size());
    int n = write(m_players[player_idx]->GetOutputFd(), msg, strlen(msg));
    if (n < 0) {
        perror("write");
    }
    return n;
}

int Match::RecvMsg(int player_idx, char *msg, int maxlen, int &exit_idx)
{
    assert(player_idx >= 0 && player_idx < m_players.size());
    fd_set read_set;
    FD_ZERO(&read_set);
    int fd, maxfd = 0;
    for (int i = 0 ; i < m_players.size() ; ++i) {
        fd = m_players[i]->GetInfoFd();
        if (fd > maxfd) maxfd = fd;
        FD_SET(fd, &read_set);
    }
    fd = m_players[player_idx]->GetInputFd();
    FD_SET(fd, &read_set);
    if (fd > maxfd) maxfd = fd;
    if (select(maxfd+1, &read_set, NULL, NULL, NULL) < 0) {
        // err
        perror("match.cpp select");
        return -1;
    }
    for (int i = 0 ; i < m_players.size() ; ++i) {
        if (FD_ISSET(m_players[i]->GetInfoFd(), &read_set)) {
            fprintf(stderr, "multiplexing: player %d exist\n", i);
            exit_idx = i;
            return 0;
        }
    }
    fprintf(stderr, "multiplexing: no player exist, before read\n");
    int n = read(m_players[player_idx]->GetInputFd(), msg, maxlen);
    msg[n] = 0;
    fprintf(stderr, "multiplexing: no player exist, after read %s\n", msg);
    return n;
}

int Match::KillPlayer(int player_idx)
{
    assert(player_idx >= 0 && player_idx < m_players.size());
    m_players[player_idx]->Kill();
}
