
#include "common.h"

#include "config.h"
#include "mpd/mpd.h"
#include "pianobar/pianobar.h"
#include "player.h"
#include "debug.h"

struct config_item players_config[] = {
#if CONFIG_PLAYER_MPD
    [PLAYER_CONFIG_MPD] = {
        .name = "mpd",
        .type = CONFIG_GROUP,
        .u.group.items = mpd_config,
        .u.group.item_count = PLAYER_CONFIG_MPD_TOTAL,
    },
#endif
#if CONFIG_PLAYER_PIANOBAR
    [PLAYER_CONFIG_PIANOBAR] = {
        .name = "pianobar",
        .type = CONFIG_GROUP,
        .u.group.items = pianobar_config,
        .u.group.item_count = PLAYER_CONFIG_PIANOBAR_TOTAL,
    },
#endif
};

