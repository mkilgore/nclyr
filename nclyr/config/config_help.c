
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "debug.h"

#define ARG_LEN 30

void config_disp_small_helptext(struct root_config *root, struct arg_parser *parser)
{
    const struct arg *a;
    printf("Usage: %s %s \n"
           "\n"
           "%s"
           "Flags:\n", parser->argv[0], parser->arg_str, parser->arg_desc_str);

    for (a = parser->args; a->lng || a->shrt; a++) {
        printf("  ");
        if (a->shrt != '\0')
            printf("-%c%c ", a->shrt, a->lng? ',': ' ');
        else
            printf("    ");

        if (a->lng && !a->has_arg) {
            printf("--%-" Q(ARG_LEN) "s ", a->lng);
        } else if (a->lng) {
            size_t len = ARG_LEN - strlen(a->lng) - 1;
            printf("--%s=%-*s ", a->lng, len, a->arg_txt);
        } else {
            printf("  %" Q(ARG_LEN) "s ", "");
        }

        printf("%s\n", a->help_txt);
    }
}

static void config_print_help_item(struct config_item *item, const char *id)
{
    const char *str;
    size_t len = ARG_LEN - strlen(item->name) - (id? strlen(id) + 1: 0) - 1;

    if (id)
        printf("      --%s-%s=", id, item->name);
    else
        printf("      --%s=", item->name);

    switch (item->type) {
    case CONFIG_STRING:
        str = "String";
        break;
    case CONFIG_BOOL:
        str = "Bool";
        break;
    case CONFIG_INTEGER:
        str = "Integer";
        break;
    case CONFIG_COLOR_PAIR:
        str = "Color Pair";
        break;
    case CONFIG_GROUP:
        /* Shouldn't happen */
        break;
    }
    printf("%-*s %s\n", len, str, item->description);
}

static void config_print_group(struct config_item *item, const char *id)
{
    char name[ARG_LEN + 1];
    struct config_item *item2;
    int i;

    if (id)
        sprintf(name, "%s-%s", id, item->name);
    else
        strcpy(name, item->name);

    for (i = 0; i < item->u.group.item_count; i++) {
        item2 = item->u.group.items + i;
        if (item2->type != CONFIG_GROUP)
            config_print_help_item(item2, name);
        else
            config_print_group(item2, name);
    }
}

void config_disp_full_helptext(struct root_config *root, struct arg_parser *parser)
{
    int i;
    struct config_item *item;
    config_disp_small_helptext(root, parser);

    for (i = 0; i < root->group.item_count; i++) {
        item = root->group.items + i;
        if (item->type != CONFIG_GROUP)
            config_print_help_item(item, NULL);
        else
            config_print_group(item, NULL);
    }

}

