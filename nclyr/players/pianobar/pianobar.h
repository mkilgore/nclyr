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

#endif
