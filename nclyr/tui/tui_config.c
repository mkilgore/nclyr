
#include "common.h"

#include "config.h"
#include "tui.h"
#include "debug.h"

static struct config_item tui_statusline[] = {
    [TUI_CONFIG_STATUSLINE_SONG] = { .name = "song", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_DEFAULT_STATUSLINE
        },
    },
};

static struct config_item tui_playlist[] = {
    [TUI_CONFIG_PLAYLIST_SEL_PLAYING] = { .name = "selplaying", .type = CONFIG_STRING,
        .u.str = {
             .str = CONFIG_TUI_DEFAULT_PLAYLIST_SEL_PLAYING
        },
    },
    [TUI_CONFIG_PLAYLIST_PLAYING] = { .name = "playing", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_DEFAULT_PLAYLIST_PLAYING
        },
    },
    [TUI_CONFIG_PLAYLIST_SEL] = { .name = "sel", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_DEFAULT_PLAYLIST_SEL
        },
    },
    [TUI_CONFIG_PLAYLIST_NORMAL] = { .name = "normal", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_DEFAULT_PLAYLIST_NORMAL
        },
    }
};

struct config_item tui_conf[] = {
    [TUI_CONFIG_PLAYLIST] = {
        .name = "playlist",
        .type = CONFIG_GROUP,
        .u.group.items = tui_playlist,
        .u.group.item_count = ARRAY_SIZE(tui_playlist),
    },
    [TUI_CONFIG_STATUSLINE] = {
        .name = "statusline",
        .type = CONFIG_GROUP,
        .u.group.items = tui_statusline,
        .u.group.item_count = ARRAY_SIZE(tui_statusline),
    },
};

struct config_item *tui_config_get_root(void)
{
    return tui_conf;
}

