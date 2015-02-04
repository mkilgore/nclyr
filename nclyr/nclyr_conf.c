
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "config.h"
#include "nclyr_conf.h"
#include "tui.h"
#include "debug.h"

struct root_config nclyr_config;

static struct config_item *global_config;

void nclyr_global_init(struct config_item *);

static void (*config_init[])(struct config_item *) = {
    nclyr_global_init,
#if CONFIG_TUI
    tui_config_init,
#endif
};

void nclyr_global_init(struct config_item *root)
{
    struct config_item globals[] = {
        [NCLYR_CONFIG_PLAYER] = { .name = "player", .type = CONFIG_STRING, .u.str = strdup(CONFIG_DEFAULT_PLAYER) },
        [NCLYR_CONFIG_INTERFACE] = { .name = "interface", .type = CONFIG_STRING, .u.str = strdup(CONFIG_DEFAULT_IFACE) },
    };

    global_config = root;

    root->name = "global";
    root->description = "Program global settings";
    root->type = CONFIG_GROUP;

    root->u.group.item_count = sizeof(globals)/sizeof(*globals);
    root->u.group.items = malloc(sizeof(globals));
    memcpy(root->u.group.items, globals, sizeof(globals));
}

static void nclyr_root_init(struct root_config *root)
{
    struct config_item root_opts[] = {
        [NCLYR_CONFIG_PLAYER] = {
            .name = "player",
            .type = CONFIG_STRING,
            .u.str = strdup(CONFIG_DEFAULT_PLAYER),
            .description = "Name of the music player to use. Use the '--list-players' flag to see full list of options",
        },
        [NCLYR_CONFIG_INTERFACE] = {
            .name = "interface",
            .type = CONFIG_STRING,
            .u.str = strdup(CONFIG_DEFAULT_IFACE),
            .description = "Name of the interface to use. Use the '--list-interfaces' flag to see full list of options",
        },
    };

    root->group.item_count = ARRAY_SIZE(root_opts);
    root->group.items = malloc(sizeof(root_opts));
    memcpy(root->group.items, root_opts, sizeof(root_opts));
}

void nclyr_conf_clear(void)
{
    int i;
    for (i = 0; i < nclyr_config.group.item_count; i++)
        config_item_clear(nclyr_config.group.items + i);
    free(nclyr_config.group.items);
}

void nclyr_conf_init(void)
{
    int i, start;

    nclyr_root_init(&nclyr_config);

    start = nclyr_config.group.item_count;

    nclyr_config.group.item_count += ARRAY_SIZE(config_init);
    nclyr_config.group.items = realloc(nclyr_config.group.items, sizeof(*nclyr_config.group.items) * nclyr_config.group.item_count);

    for (i = 0; i < ARRAY_SIZE(config_init); i++)
        (config_init[i]) (nclyr_config.group.items + i + start);
}

struct config_item *nclyr_global_conf(void)
{
    return global_config;
}

struct config_item *nclyr_conf_get(const char *str)
{
    return config_item_find(&nclyr_config, str);
}

