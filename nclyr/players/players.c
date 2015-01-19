
#include "common.h"

#include "player.h"
#include "pianobar/pianobar.h"

struct player *players[] = {
#ifdef CONFIG_PLAYER_PIANOBAR
    &pianobar_player.player,
#endif
    NULL
};

struct player *player_current_used(void)
{
    return players[0];
}

void player_setup_notification(int pipefd)
{
    players[0]->start_monitor(pipefd);
    return ;
}

void player_stop_notification(void)
{
    players[0]->stop_monitor();
    return ;
}

void player_notification_free(struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_NO_SONG:

        break;
    case PLAYER_SONG:
        song_clear(&notif->u.song);
        break;
    }
}

