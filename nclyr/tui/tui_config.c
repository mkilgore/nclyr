
#include "common.h"

#include "config.h"
#include "tui.h"
#include "debug.h"

static struct config_item *tui_root;

void tui_config_init(struct config_item *root)
{
    struct config_item tui_playlist[] = {
        [TUI_CONFIG_PLAYLIST_SEL_PLAYING] = { .name = "selplaying", .type = CONFIG_STRING,
            .u.str = strdup(
                        "${bold}${play-num;w:2}. ${reverse}${color;f:cyan}${title}${color;f:default} | ${color;f:red}${artist}${color;f:default} | ${color;f:green}${album}"
                    ),
        },
        [TUI_CONFIG_PLAYLIST_PLAYING] = { .name = "playing", .type = CONFIG_STRING,
            .u.str = strdup(
                        "${bold}${play-num;w:2}. ${color;f:cyan}${title}${color;f:default} | ${color;f:red}${artist}${color;f:default} | ${color;f:green}${album}"
                    ),
        },
        [TUI_CONFIG_PLAYLIST_SEL] = { .name = "sel", .type = CONFIG_STRING,
            .u.str = strdup(
                        "${play-num;w:2}. ${reverse}${color;f:cyan}${title}${color;f:default} | ${color;f:red}${artist}${color;f:default} | ${color;f:green}${album}"
                    ),
        },
        [TUI_CONFIG_PLAYLIST_NORMAL] = { .name = "normal", .type = CONFIG_STRING,
            .u.str = strdup(
                        "${play-num;w:2}. ${color;f:cyan}${title}${color;f:default} | ${color;f:red}${artist}${color;f:default} | ${color;f:green}${album}"
                    ),
        }
    };
    struct config_item tui_statusline[] = {
        [TUI_CONFIG_STATUSLINE_SONG] = { .name = "song", .type = CONFIG_STRING,
            .u.str = strdup(
                        " ${color;f:cyan;b:default}${title}${color;f:default;b:default} | ${color;f:red;b:default}${artist}${color;f:default;b:default} | ${color;f:green;b:default}${album}"
                    ),
        },
    };
    struct config_item tui[] = {
        [TUI_CONFIG_PLAYLIST] = { .name = "playlist", .type = CONFIG_GROUP },
        [TUI_CONFIG_STATUSLINE] = { .name = "statusline", .type = CONFIG_GROUP },
    };
    struct config_item *item;

    tui_root = root;
    root->name = "tui";
    root->description = "Tui interface settings";
    root->type = CONFIG_GROUP;

    root->u.group.item_count = ARRAY_SIZE(tui);
    root->u.group.items = malloc(sizeof(tui));
    memcpy(root->u.group.items, tui, sizeof(tui));

    item = root->u.group.items + TUI_CONFIG_PLAYLIST;

    item->u.group.item_count = ARRAY_SIZE(tui_playlist);
    item->u.group.items = malloc(sizeof(tui_playlist));
    memcpy(item->u.group.items, tui_playlist, sizeof(tui_playlist));

    item = root->u.group.items + TUI_CONFIG_STATUSLINE;

    item->u.group.item_count = ARRAY_SIZE(tui_statusline);
    item->u.group.items = malloc(sizeof(tui_statusline));
    memcpy(item->u.group.items, tui_statusline, sizeof(tui_statusline));
}

struct config_item *tui_config_get_root(void)
{
    return tui_root;
}

