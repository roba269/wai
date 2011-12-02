#include <cassert>
#include <cstring>
#include <sys/select.h>
#include "match.h"
#include "player.h"

int Match::SendMsg(int player_idx, const char *msg)
{
    assert(player_idx >= 0 && player_idx < m_players.size());
    return write(m_players[player_idx]->GetInputFd(), msg, strlen(msg));
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
    fd = m_players[player_idx]->GetOutputFd();
    FD_SET(fd, &read_set);
    if (fd > maxfd) maxfd = fd;
    if (select(maxfd+1, &read_set, NULL, NULL, NULL) < 0) {
        // err
        return -1;
    }
    for (int i = 0 ; i < m_players.size() ; ++i) {
        if (FD_ISSET(m_players[i]->GetInfoFd(), &read_set)) {
            exit_idx = i;
            return 0;
        }
    }
    int n = read(m_players[player_idx]->GetOutputFd(), msg, maxlen);
    msg[n] = 0;
    return n;
}

int Match::KillPlayer(int player_idx)
{
    assert(player_idx >= 0 && player_idx < m_players.size());
    m_players[player_idx]->Kill();
}

