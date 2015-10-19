
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
    [TUI_CONFIG_STATUSLINE_SONG_TRIPLE] = { .name = "song_triple", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_STATUSLINE_SONG_TRIPLE_DEFAULT
        },
    },
    [TUI_CONFIG_STATUSLINE_SONG_FILENAME] = { .name = "song_filename", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_STATUSLINE_SONG_FILENAME_DEFAULT
        },
    },
};

static struct config_item tui_playlist[] = {
    [TUI_CONFIG_PLAYLIST_PRINTLINE] = { .name = "printline", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_PLAYLIST_PRINTLINE_DEFAULT
        },
    },
    [TUI_CONFIG_PLAYLIST_SONG_TRIPLE] = { .name = "song_triple", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_PLAYLIST_SONG_TRIPLE_DEFAULT
        },
    },
    [TUI_CONFIG_PLAYLIST_SONG_FILENAME] = { .name = "song_filename", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_PLAYLIST_SONG_FILENAME_DEFAULT
        },
    },
};

#if CONFIG_TUI_MPD_VISUALIZER
static struct config_item tui_mpd_visualizer[] = {
    [TUI_CONFIG_MPD_VISUALIZER_FILENAME] = { .name = "filename", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_TUI_MPD_VISUALIZER_FILENAME_DEFAULT
        },
    },
    [TUI_CONFIG_MPD_VISUALIZER_COLOR] = { .name = "color", .type = CONFIG_COLOR_PAIR,
        .u.c_pair = CONFIG_TUI_MPD_VISUALIZER_COLOR_DEFAULT,
    },
};
#endif

#if CONFIG_PLAYER_MPD
static struct config_item tui_mpd[] = {
# if CONFIG_TUI_MPD_VISUALIZER
    [TUI_CONFIG_MPD_VISUALIZER] = {
        .name = "visualizer",
        .type = CONFIG_GROUP,
        .u.group.items = tui_mpd_visualizer,
        .u.group.item_count = ARRAY_SIZE(tui_mpd_visualizer),
    },
# endif
};
#endif

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
#if CONFIG_PLAYER_MPD
    [TUI_CONFIG_MPD] = {
        .name = "mpd",
        .type = CONFIG_GROUP,
        .u.group.items = tui_mpd,
        .u.group.item_count = ARRAY_SIZE(tui_mpd),
    },
#endif
};

struct config_item *tui_config_get_root(void)
{
    return tui_conf;
}

