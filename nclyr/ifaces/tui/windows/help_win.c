
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "tui_internal.h"
#include "player.h"
#include "lyr_thread.h"
#include "window.h"
#include "char_to_str.h"
#include "help_win.h"

static int help_count_lines(struct tui_iface *tui)
{
    int lines = 2, i;
    const struct nclyr_keypress *key;

    for (key = tui->global_keys; key->ch != '\0'; key++)
        lines++;

    for (i = 0; i < tui->window_count; i++) {
        struct nclyr_win *win = tui->windows[i];

        lines += 4;
        for (key = win->keypresses; key->ch != '\0'; key++)
            lines++;
    }

    return lines;
}

static char *help_create_dash_line(int width)
{
    char *line = malloc(width * sizeof(char) + 1);
    memset(line, 0, width + 1);

    memset(line, ' ', 3);
    memset(line + 3, '-', width - 6);
    memset(line + width - 3, ' ', 3);

    return line;
}

static void help_resize_dashes(struct line_win *win)
{
    int l;
    for (l = 0; l < win->line_count; l++) {
        if (strncmp(win->lines[l], "   -", 4) == 0) {
            free(win->lines[l]);
            win->lines[l] = help_create_dash_line(COLS);
        }
    }
}

static int help_create_key_text(char **lines, const char *title, const struct nclyr_keypress *keys)
{
    int i, l_count = 0;
    a_sprintf(&lines[0], "      %s", title);
    l_count++;

    lines[1] = help_create_dash_line(COLS);
    l_count++;

    for (i = 0; keys[i].ch != '\0'; i++) {
        a_sprintf(lines + i + 2, "   %12s : %s", keypress_to_str(keys + i), keys[i].help_text);
        l_count++;
    }

    return l_count;
}

static void help_create_text(struct tui_iface *tui, struct line_win *line)
{
    int i;
    int next;

    next = help_create_key_text(line->lines, "Global", tui->global_keys);

    for (i = 0; i < tui->window_count; i++) {
        struct nclyr_win *win = tui->windows[i];

        line->lines[next++] = strdup("");
        line->lines[next++] = strdup("");
        next += help_create_key_text(line->lines + next, win->win_name, win->keypresses);
    }
}

static void help_init(struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    struct tui_iface *tui = win->tui;

    line->line_count = help_count_lines(tui);
    line->disp_offset = 0;

    line->lines = malloc(line->line_count * sizeof(*line->lines));

    help_create_text(tui, line);
}

static void help_resize(struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    help_resize_dashes(line);
}

static struct line_win help_window_init = {
    .super_win = {
        .win_name = "Help",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            LINE_KEYPRESSES(),
            N_END()
        },
        .init = help_init,
        .clean = line_clean,
        .switch_to = NULL,
        .update = line_update,
        .resize = help_resize,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = NULL,
    }
};

struct nclyr_win *help_win_new(void)
{
    struct line_win *win = malloc(sizeof(*win));
    memcpy(win, &help_window_init, sizeof(help_window_init));
    return &win->super_win;
}

