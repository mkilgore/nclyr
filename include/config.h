#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include <ncurses.h>

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
    char *str;
    int bol;
    int integer;

    struct {
        int f;
        int b;
    } c_pair;

    struct item_group group;
};

struct config_item {
    const char *name;
    const char *description;
    enum config_item_type type;
    union config_data u;
};

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

int config_color_get(const char *name);
const char *config_color_name(int color);

void config_check_for_config(int argc, const char **argv, const char **config);

void config_print(struct root_config *);
int config_load_from_file(struct root_config *, const char *file);
int config_load_from_args(struct root_config *, struct arg_parser *);

void config_disp_small_helptext(struct root_config *, struct arg_parser *);
void config_disp_full_helptext(struct root_config *, struct arg_parser *);

#endif
