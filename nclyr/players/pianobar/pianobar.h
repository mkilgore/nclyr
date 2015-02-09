#ifndef PLAYERS_PIANOBAR_H
#define PLAYERS_PIANOBAR_H

#include <pthread.h>

#include "player.h"
#include "song.h"

struct pianobar_player {
    struct player player;
    struct song_info *current_song;
    int stop_pipe[2];
    pthread_t notif_thread;
};

extern struct pianobar_player pianobar_player;

enum {
    PLAYER_CONFIG_PIANOBAR_NOWPLAYING,
    PLAYER_CONFIG_PIANOBAR_FIFO,
    PLAYER_CONFIG_PIANOBAR_TOTAL
};

extern struct config_item pianobar_config[];

#endif
