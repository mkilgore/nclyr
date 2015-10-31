
#include "common.h"

#include "config.h"
#include "nclyr_conf.h"
#include "tui_internal.h"
#include "window.h"
#include "line_win.h"
#include "config_win.h"
#include "debug.h"

static void add_line(struct line_win *line, const char *line_str)
{
    line->line_count++;
    line->lines = realloc(line->lines, sizeof(*line->lines) * (line->line_count));

    line->lines[line->line_count - 1] = strdup(line_str);
}

static void config_add_group(struct line_win *line, struct item_group *group, int ident)
{
    char line_buf[200];
    int i;

    for (i = 0; i < group->item_count; i++) {
        struct config_item *item = group->items + i;

        if (item->type == CONFIG_GROUP) {
            snprintf(line_buf, sizeof(line_buf), "%*s %s:", ident * 4, "", item->name);
            add_line(line, line_buf);
            config_add_group(line, &item->u.group, ident + 1);
            continue;
        }

        snprintf(line_buf, sizeof(line_buf), "%*s %s:", ident * 4, "", item->name);
        add_line(line, line_buf);
        switch (item->type) {
        case CONFIG_GROUP:
            break;

        case CONFIG_STRING:
            snprintf(line_buf, sizeof(line_buf), "%*s   Value: \"%s\"", ident * 4, "", item->u.str.str);
            break;

        case CONFIG_INTEGER:
            snprintf(line_buf, sizeof(line_buf), "%*s   Value: %d", ident * 4, "", item->u.integer);
            break;

        case CONFIG_BOOL:
            snprintf(line_buf, sizeof(line_buf), "%*s   Value: %s", ident * 4, "", (item->u.bol)? "true": "false");
            break;

        case CONFIG_COLOR_PAIR:
            snprintf(line_buf, sizeof(line_buf), "%*s   Value: (%s, %s)", ident * 4, "", cons_color_name(item->u.c_pair.f), cons_color_name(item->u.c_pair.b));
            break;
        }
        add_line(line, line_buf);
    }
}

static void config_init(struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    config_add_group(line, &nclyr_config.group, 0);
}

static struct line_win config_window_init = {
    .super_win = {
        .win_name = "Config",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            LINE_KEYPRESSES(),
            NCLYR_END()
        },
        .init = config_init,
        .clean = line_clean,
        .switch_to = NULL,
        .update = line_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = NULL,
    }
};

struct nclyr_win *config_win_new(void)
{
    struct line_win *win = malloc(sizeof(*win));
    memcpy(win, &config_window_init, sizeof(config_window_init));
    return &win->super_win;
}

