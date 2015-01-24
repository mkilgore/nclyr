
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "tui.h"
#include "tui_internal.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui/window.h"
#include "help_win.h"

static int help_count_lines(void)
{
    int lines = 0;
    struct nclyr_win **win;
    const struct nclyr_keypress *key;

    lines += 1;

    for (key = tui.global_keys; key->ch != '\0'; key++)
        lines++;

    for (win = tui.windows; *win; win++) {
        lines += 2;
        for (key = (*win)->keypresses; key->ch != '\0'; key++)
            lines++;
        lines += 2;
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
        a_sprintf(lines + i + 2, "      %c : %s", keys[i].ch, keys[i].help_text);
        l_count++;
    }
    lines[l_count++] = strdup("");
    lines[l_count++] = strdup("");

    return l_count;
}

static void help_create_text(struct line_win *line)
{
    struct nclyr_win **win;
    int next;
    next = help_create_key_text(line->lines, "Global", tui.global_keys) - 1;
    for (win = tui.windows; *win; win++)
        next += help_create_key_text(line->lines + next, (*win)->win_name, (*win)->keypresses) - 1;
}

static void help_init(struct nclyr_win *win, int y, int x, int rows, int cols)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    win->win = newwin(rows, cols, y, x);

    line->line_count = help_count_lines();
    line->disp_offset = 0;

    line->lines = malloc(line->line_count * sizeof(*line->lines));

    help_create_text(line);
}

static void help_resize(struct nclyr_win *win, int y, int x, int rows, int cols)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    line_resize(win, y, x, rows, cols);

    help_resize_dashes(line);
}

struct line_win help_window = {
    .super_win = {
        .win_name = "Help",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            LINE_KEYPRESSES,
            { '\0', NULL, NULL }
        },
        .init = help_init,
        .clean = line_clean,
        .switch_to = line_switch_to,
        .update = line_update,
        .resize = help_resize,
        .clear_song_data = line_clear_song_data,
        .new_song_data = NULL,
        .new_player_notif = line_new_player_notif,
    }
};

