#ifndef INCLUDE_PLAYERS_PIANOBAR_H
#define INCLUDE_PLAYERS_PIANOBAR_H

#include "config.h"

enum {
    PLAYER_PIANOBAR_CONFIG_NOWPLAYING,
    PLAYER_PIANOBAR_CONFIG_FIFO,
    PLAYER_PIANOBAR_CONFIG_TOTAL
};

extern struct config_item pianobar_config[];

#endif
