
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "song.h"
#include "playlist.h"
#include "player.h"
#include "pianobar/pianobar.h"
#include "mpd/mpd.h"
#include "debug.h"

struct player *players[] = {
#if CONFIG_PLAYER_PIANOBAR
    &pianobar_player.player,
#endif
#if CONFIG_PLAYER_MPD
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

