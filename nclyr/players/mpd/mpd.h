#ifndef PLAYERS_PIANOBAR_H
#define PLAYERS_PIANOBAR_H

#include "player.h"

struct mpd_player {
    struct player player;
};

extern struct mpd_player mpd_player;

#endif
