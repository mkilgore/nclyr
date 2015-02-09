
#include "common.h"

#include "config.h"
#include "mpd.h"
#include "debug.h"

struct config_item mpd_config[] = {
    [PLAYER_CONFIG_MPD_SERVER] = {
        .name = "server",
        .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_PLAYER_MPD_SERVER_DEFAULT
        },
    },
    [PLAYER_CONFIG_MPD_PORT] = {
        .name = "port",
        .type = CONFIG_INTEGER,
        .u.integer = CONFIG_PLAYER_MPD_PORT_DEFAULT,
    },
};

