
#include "common.h"

#include "config.h"
#include "tui_internal.h"
#include "debug.h"

static struct config_item tui_statusline[] = {
    [TUI_CONFIG_STATUSLINE_SONG] = { .name = "song", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_STATUSLINE_SONG_DEFAULT
        },
    },
};

static struct config_item tui_playlist[] = {
    [TUI_CONFIG_PLAYLIST_PRINTLINE] = { .name = "printline", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_PLAYLIST_PRINTLINE_DEFAULT
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

