
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stringcasecmp.h"
#include "cons_color.h"
#include "config.h"
#include "debug.h"

static void print_indent(int indent)
{
    int i;
    for (i = 0; i < indent; i++)
        printf("    ");
}

static void config_print_item(struct config_item *item, int indent)
{
    int i;

    print_indent(indent);

    switch (item->type) {
    case CONFIG_GROUP:
        printf("%s:\n", item->name);
        for (i = 0; i < item->u.group.item_count; i++)
            config_print_item(item->u.group.items + i, indent + 1);
        break;

    case CONFIG_STRING:
        printf("%s: %s\n", item->name, item->u.str);
        break;

    case CONFIG_INTEGER:
        printf("%s: %d\n", item->name, item->u.integer);
        break;

    case CONFIG_BOOL:
        printf("%s: %s\n", item->name, (item->u.bol)? "true": "false");
        break;

    case CONFIG_COLOR_PAIR:
        printf("%s: (%s, %s)\n", item->name, cons_color_name(item->u.c_pair.f), cons_color_name(item->u.c_pair.b));
        break;
    }
}

void config_print(struct root_config *root)
{
    int i;
    printf("Configuration:\n");
    for (i = 0; i < root->group.item_count; i++)
        config_print_item(root->group.items + i, 0);
}

