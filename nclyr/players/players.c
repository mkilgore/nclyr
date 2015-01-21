
#include "common.h"

#include <string.h>

#include "player.h"
#include "pianobar/pianobar.h"
#include "mpd/mpd.h"

struct player *players[] = {
#ifdef CONFIG_PLAYER_PIANOBAR
    &pianobar_player.player,
#endif
#ifdef CONFIG_PLAYER_MPD
    &mpd_player.player,
#endif
    NULL
};

static struct player *sel_player = NULL;

struct player *player_find(const char *name)
{
    struct player **player;
    for (player = players; *player; player++)
        if (strcmp((*player)->name, name) == 0)
            return *player;

    return NULL;
}

void player_set_current(struct player *player)
{
    sel_player = player;
}

struct player *player_current(void)
{
    return sel_player;
}

void player_start_thread(struct player *player, int pipefd)
{
    if (!player)
        return ;

    player->start_thread(player, pipefd);
    return ;
}

void player_stop_thread(struct player *player)
{
    if (!player)
        return ;

    player->stop_thread(player);
    return ;
}

void player_notification_free(struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_SONG:
        song_clear(&notif->u.song);
        break;

    case PLAYER_NO_SONG:
    case PLAYER_IS_DOWN:
    case PLAYER_IS_UP:
        break;
    }
}

