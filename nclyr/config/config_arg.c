
#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "stringcasecmp.h"
#include "config.h"
#include "debug.h"

static int send_extra_arg(struct arg_parser *parser, int arg)
{
    return (parser->arg_callback)(parser, ARG_PARSER_EXTRA, parser->argv[arg]);
}

#define NEXT_ARG(parser, arg, opt) \
    do { \
        if ((parser)->argc == (arg) + 1) { \
            printf("%s: Not enough arguments to '--%s'\n", (parser)->argv[0], (opt)); \
            return 1; \
        } \
        (arg)++; \
    } while (0)

static int parse_long(struct root_config *root, struct arg_parser *parser, int *arg)
{
    int i;
    const char *opt = parser->argv[*arg] + 2;
    const struct arg *args = parser->args;
    struct config_item *item = config_item_find(root, opt);

    if (item != NULL) {
        char color1[10], color2[10];
        NEXT_ARG(parser, *arg, opt);
        switch (item->type) {
        case CONFIG_STRING:
            printf("Found argument for arg '%s' -> '%s'\n", item->name, parser->argv[*arg]);
            config_item_data_clear(item->type, &item->u);
            rd_string_dup(&item->u.str, parser->argv[*arg]);
            break;
        case CONFIG_INTEGER:
            config_item_data_clear(item->type, &item->u);
            item->u.integer = strtol(parser->argv[*arg], NULL, 0);
            break;
        case CONFIG_BOOL:
            config_item_data_clear(item->type, &item->u);
            item->u.bol = (stringcasecmp(parser->argv[*arg], "true") == 0);
            break;
        case CONFIG_COLOR_PAIR:
            if (sscanf(parser->argv[*arg], "(%9[^(,],%9[^,)])", color1, color2) != 2) {
                printf("%s: Error parsing color pair '%s'\n", parser->argv[0], parser->argv[*arg]);
                return 1;
            }
            printf("colors: %s, %s\n", color1, color2);
            config_item_data_clear(item->type, &item->u);
            item->u.c_pair.f = cons_color_get(color1);
            item->u.c_pair.b = cons_color_get(color2);
            break;
        default:
            break;
        }
        return 0;
    } else {
        for (i = 0; args[i].lng != NULL || args[i].shrt; i++) {
            if (strcmp(args[i].lng, opt) == 0) {
                if (args[i].has_arg) {
                    if (parser->argc == *arg + 1) {
                        printf("%s: Not enough arguments to '--%s'\n", parser->argv[0], opt);
                        return 1;
                    }
                    (*arg)++;
                    if ((parser->arg_callback) (parser, i, parser->argv[*arg]) != 0)
                        return 1;
                } else {
                    if ((parser->arg_callback) (parser, i, NULL) != 0)
                        return 1;
                }
                return 0;
            }
        }
    }
    return 1;
}

static int parse_short(struct root_config *root, struct arg_parser *parser, int *arg)
{
    int i;
    const char *cur;
    const struct arg *args = parser->args;

    for (cur = parser->argv[*arg] + 1; *cur; cur++) {
        for (i = 0; args[i].lng != NULL || args[i].shrt; i++) {
            if (args[i].shrt == *cur) {
                if (args[i].has_arg) {
                    if (parser->argc == *arg + 1) {
                        printf("%s: Not enough arguments to '-%c'\n", parser->argv[0], *cur);
                        return 1;
                    }
                    (*arg)++;
                    if ((parser->arg_callback) (parser, i, parser->argv[*arg]) != 0)
                        return 1;
                    return 0;
                } else {
                    if ((parser->arg_callback) (parser, i, NULL) != 0)
                        return 1;
                }
                break;
            }
        }
    }
    return 0;
}

int config_load_from_args(struct root_config *root, struct arg_parser *parser)
{
    int current_arg = 0;
    int parsing_done = 0;

    while ((++current_arg) != parser->argc) {
        const char *cur = parser->argv[current_arg];
        if (parsing_done)
            if (send_extra_arg(parser, current_arg) != 0)
                return 1;

        if (cur[0] == '-') {
            if (cur[1] == '-') {
                if (cur[2] == '\0') {
                    /* Got sequence '--', stop parsing the rest of the arguments */
                    parsing_done = 1;
                } else {
                    /* Check for long option */
                    if (parse_long(root, parser, &current_arg) != 0)
                        return 1;
                }
            } else {
                /* Check for short option */
                if (parse_short(root, parser, &current_arg) != 0)
                    return 1;
            }
        } else {
            if (send_extra_arg(parser, current_arg) != 0)
                return 1;
        }
    }

    return 0;
}

void config_check_for_config(int argc, const char **argv, const char **file)
{

}

