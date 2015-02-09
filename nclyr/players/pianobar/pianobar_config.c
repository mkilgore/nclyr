
#include "common.h"

#include "config.h"
#include "pianobar.h"
#include "players/pianobar.h"
#include "debug.h"

struct config_item pianobar_config[] = {
    [PLAYER_PIANOBAR_CONFIG_NOWPLAYING] = {
        .name = "nowplaying",
        .type = CONFIG_STRING,
        .u.str = {
            .str = "~/.config/pianobar/nowplaying"
        },
    },
    [PLAYER_PIANOBAR_CONFIG_FIFO] = {
        .name = "fifo",
        .type = CONFIG_STRING,
        .u.str = {
            .str = "~/.config/pianobar/ctl"
        },
    },
};

