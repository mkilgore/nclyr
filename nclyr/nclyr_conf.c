
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "config.h"
#include "nclyr_conf.h"
#include "tui.h"
#include "console.h"
#include "debug.h"

static struct config_item global_conf[] = {
    [NCLYR_CONFIG_PLAYER] = {
        .name = "player",
        .type = CONFIG_STRING,
        .description = "Player to connect too",
        .u.str = { .str = CONFIG_DEFAULT_PLAYER },
    },
    [NCLYR_CONFIG_INTERFACE] = {
        .name = "interface",
        .type = CONFIG_STRING,
        .description = "Interface to use",
        .u.str = { .str = CONFIG_DEFAULT_IFACE },
    },
#if CONFIG_TUI
    [NCLYR_CONFIG_TUI] = {
        .name = "tui",
        .type = CONFIG_GROUP,
        .u.group.items = tui_conf,
        .u.group.item_count = TUI_CONFIG_TOTAL,
    },
#endif
#if CONFIG_CONSOLE
    [NCLYR_CONFIG_CONSOLE] = {
        .name = "console",
        .type = CONFIG_GROUP,
        .u.group.items = console_conf,
        .u.group.item_count = CONSOLE_CONFIG_TOTAL,
    },
#endif
};

struct root_config nclyr_config = {
    .group = {
        .items = global_conf,
        .item_count = ARRAY_SIZE(global_conf),
    },
};

void nclyr_conf_clear(void)
{
    int i;
    for (i = 0; i < nclyr_config.group.item_count; i++)
        config_item_clear(nclyr_config.group.items + i);
}

struct config_item *nclyr_global_conf(void)
{
    return global_conf;
}

struct config_item *nclyr_conf_get(const char *str)
{
    return config_item_find(&nclyr_config, str);
}

