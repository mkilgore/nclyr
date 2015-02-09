
#include "common.h"

#include "config.h"
#include "pianobar.h"
#include "debug.h"

struct config_item pianobar_config[] = {
    [PLAYER_CONFIG_PIANOBAR_NOWPLAYING] = {
        .name = "nowplaying",
        .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_PLAYER_PIANOBAR_NOWPLAYING_DEFAULT
        },
    },
    [PLAYER_CONFIG_PIANOBAR_FIFO] = {
        .name = "fifo",
        .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_PLAYER_PIANOBAR_FIFO_DEFAULT
        },
    },
};

