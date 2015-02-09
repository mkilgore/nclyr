#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include "common.h"

#include <stdlib.h>
#include "cons_color.h"

enum config_item_type {
    CONFIG_STRING,
    CONFIG_INTEGER,
    CONFIG_BOOL,
    CONFIG_COLOR_PAIR,
    CONFIG_GROUP,
};

struct config_item;

struct item_group {
    size_t item_count;
    struct config_item *items;
};

union config_data {
    struct rd_string str;
    int bol;
    int integer;

    struct cons_color_pair c_pair;

    struct item_group group;
};

struct config_item {
    const char *name;
    const char *description;
    enum config_item_type type;
    union config_data u;
};

#define CONFIG_HAS_ARGS_2(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define CONFIG_ARG_COUNT(...) CONFIG_HAS_ARGS_2(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1)

#define CONFIG_GET_ONE(root, idex) ((root + idex))

#define CONFIG_GET_1(conf, idex)             CONFIG_GET_ONE(conf, idex)
#define CONFIG_GET_2(conf, idex, idex2)      CONFIG_GET_1(CONFIG_GET_ONE(conf, idex)->u.group.items, TP(TP(idex, _), idex2))
#define CONFIG_GET_3(conf, idex, idex2, ...) CONFIG_GET_2(CONFIG_GET_ONE(conf, idex)->u.group.items, TP(TP(idex, _), idex2), __VA_ARGS__)
#define CONFIG_GET_4(conf, idex, idex2, ...) CONFIG_GET_3(CONFIG_GET_ONE(conf, idex)->u.group.items, TP(TP(idex, _), idex2), __VA_ARGS__)
#define CONFIG_GET_5(conf, idex, idex2, ...) CONFIG_GET_4(CONFIG_GET_ONE(conf, idex)->u.group.items, TP(TP(idex, _), idex2), __VA_ARGS__)
#define CONFIG_GET_6(conf, idex, idex2, ...) CONFIG_GET_5(CONFIG_GET_ONE(conf, idex)->u.group.items, TP(TP(idex, _), idex2), __VA_ARGS__)

#define CONFIG_GET(root, ...) TP(CONFIG_GET_, CONFIG_ARG_COUNT(__VA_ARGS__))(root, __VA_ARGS__)

struct root_config {
    struct item_group group;
};


struct arg {
  const char *lng;
  char shrt;
  const char *help_txt;
  const char *arg_txt;

  int has_arg :1;
};

#define ARG_PARSER_EXTRA -3
#define ARG_PARSER_ERR -2
#define ARG_PARSER_DONE -1

struct arg_parser {
    const struct arg *args;
    int (*arg_callback) (struct arg_parser *, int index, const char *arg);

    const char **argv;
    int argc;

    const char *arg_str;
    const char *arg_desc_str;
};

struct config_item *config_item_find(struct root_config *root, const char *id);
void config_item_clear(struct config_item *item);
void config_item_data_clear(enum config_item_type type, union config_data *data);

void config_check_for_config(int argc, const char **argv, const char **file);

void config_print(struct root_config *);
int config_load_from_file(struct root_config *, const char *file);
int config_load_from_args(struct root_config *, struct arg_parser *);

void config_disp_small_helptext(struct root_config *, struct arg_parser *);
void config_disp_complete_configtext(struct root_config *root);
void config_disp_root_help(struct root_config *root);

#endif
