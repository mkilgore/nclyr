
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "config.h"
#include "nclyr_conf.h"
#include "debug.h"

struct root_config nclyr_config;

void nclyr_global_init(struct config_item *);

static void (*config_init[])(struct config_item *) = {
    nclyr_global_init,
};

void nclyr_global_init(struct config_item *root)
{
    struct config_item group1[] = {
        { .name = "group_entry1", .type = CONFIG_STRING, .u.str = strdup("Stuff") },
        { .name = "group_entry2", .type = CONFIG_BOOL, .u.bol = 0 },
        { .name = "group_entry3", .type = CONFIG_INTEGER, .u.integer = 20 },
    };

    struct config_item globals[] = {
        { .name = "test", .type = CONFIG_STRING, .u.str = strdup("Default string") },
        { .name = "test2", .type = CONFIG_BOOL, .u.bol = 1 },
        { .name = "group1", .type = CONFIG_GROUP },
        { .name = "test3", .type = CONFIG_COLOR_PAIR, .u.c_pair.f = COLOR_WHITE, .u.c_pair.b = COLOR_BLUE },
    };

    struct config_item *item;

    root->name = "global";
    root->description = "Program global settings";
    root->type = CONFIG_GROUP;

    root->u.group.item_count = sizeof(globals)/sizeof(*globals);
    root->u.group.items = malloc(sizeof(globals));
    memcpy(root->u.group.items, globals, sizeof(globals));

    item = root->u.group.items + 2;

    item->u.group.item_count = sizeof(group1)/sizeof(*group1);
    item->u.group.items = malloc(sizeof(group1));
    memcpy(item->u.group.items, group1, sizeof(group1));
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
    int i;

    memset(&nclyr_config, 0, sizeof(nclyr_config));
    nclyr_config.group.item_count = sizeof(config_init)/sizeof(*config_init);
    nclyr_config.group.items = malloc(sizeof(*nclyr_config.group.items) * nclyr_config.group.item_count);

    for (i = 0; i < nclyr_config.group.item_count; i++)
        (config_init[i]) (nclyr_config.group.items + i);
}

struct config_item *nclyr_conf_get(const char *str)
{
    return config_item_find(&nclyr_config, str);
}

