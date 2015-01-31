
#include "common.h"

#include "player.h"
#include "debug.h"

void player_start_thread(struct player *player, int pipefd)
{
    if (!player)
        return ;

    player->notify_fd = pipefd;
    player->start_thread(player);
    return ;
}

void player_stop_thread(struct player *player)
{
    if (!player)
        return ;

    player->stop_thread(player);
    return ;
}

