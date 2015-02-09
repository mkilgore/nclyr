
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cons_color.h"
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
            int len = ARG_LEN - strlen(a->lng) - 1;
            printf("--%s=%-*s ", a->lng, len, a->arg_txt);
        } else {
            printf("  %" Q(ARG_LEN) "s ", "");
        }

        printf("%s\n", a->help_txt);
    }
}

static const char *config_opt_str(enum config_item_type type)
{
    const char *type_strs[] = {
        [CONFIG_STRING] = "String",
        [CONFIG_BOOL] = "Bool",
        [CONFIG_INTEGER] = "Integer",
        [CONFIG_COLOR_PAIR] = "Color Pair",
    };

    return type_strs[type];
}

static void config_print_help_item(struct config_item *item, const char *id)
{
    int len = ARG_LEN - strlen(item->name) - (id? strlen(id) + 1: 0) - 1;

    if (id)
        printf("      --%s-%s=", id, item->name);
    else
        printf("      --%s=", item->name);

    printf("%-*s %s\n", len, config_opt_str(item->type), item->description);
}

static void config_print_help_item_complete(struct config_item *item, const char *id)
{
    if (id)
        printf(" %s-%s\n", id, item->name);
    else
        printf(" %s\n", item->name);
    printf("    Type: %s\n", config_opt_str(item->type));
    printf("    Value: ");
    switch (item->type) {
    case CONFIG_STRING:
        if (item->u.str.str)
            printf("\"%s\"\n", item->u.str.str);
        else
            printf("Empty\n");
        break;
    case CONFIG_BOOL:
        printf("%s\n", (item->u.bol)?"True":"False");
        break;
    case CONFIG_INTEGER:
        printf("%d\n", item->u.integer);
        break;
    case CONFIG_COLOR_PAIR:
        printf("(%s, %s)\n", cons_color_name(item->u.c_pair.f), cons_color_name(item->u.c_pair.b));
        break;
    case CONFIG_GROUP:
        /* Shouldn't happen */
        break;
    }
    printf("    Description: %s\n", item->description);
    putchar('\n');
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
        if (item2->type != CONFIG_GROUP) {
            config_print_help_item_complete(item2, name);
        } else {
            config_print_group(item2, name);
        }
    }
}

void config_disp_complete_configtext(struct root_config *root)
{
    int i;
    struct config_item *item;

    for (i = 0; i < root->group.item_count; i++) {
        item = root->group.items + i;
        if (item->type != CONFIG_GROUP) {
            config_print_help_item_complete(item, NULL);
        } else {
            config_print_group(item, NULL);
        }
    }
}

void config_disp_root_help(struct root_config *root)
{
    int i;
    for (i = 0; i < root->group.item_count; i++)
        if (root->group.items[i].type != CONFIG_GROUP)
            config_print_help_item(root->group.items + i, NULL);
}

