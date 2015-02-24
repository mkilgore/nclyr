
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <time.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "player.h"
#include "tui_internal.h"
#include "window.h"
#include "tui_color.h"
#include "clock_win.h"

static char big_num[15][5][3] = {
    [0] = {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 0, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 }
    },
    [1] = {
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 }
    },
    [2] = {
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 1, 1, 1 },
        { 1, 0, 0 },
        { 1, 1, 1 }
    },
    [3] = {
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    },
    [4] = {
        { 1, 0, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 }
    },
    [5] = {
        { 1, 1, 1 },
        { 1, 0, 0 },
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    },
    [6] = {
        { 1, 1, 1 },
        { 1, 0, 0 },
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 }
    },
    [7] = {
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 }
    },
    [8] = {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 }
    },
    [9] = {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    },
    [10] = { /* P */
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 1, 0, 0 },
        { 1, 0, 0 }
    },
    [11] = { /* M */
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 0, 1 },
        { 1, 0, 1 }
    },
    [12] = { /* A */
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 1, 0, 1 }
    },
    [13] = { /* colon */
        { 0, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 0 }
    },
    [14] = { /* space */
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 }
    }
};

static inline void clock_strlen(const char *str, int *width, int *height)
{
    int len = strlen(str);

    *width = len * (sizeof(**big_num) / sizeof(***big_num) + 1) - 1;
    *height = (sizeof(*big_num) / sizeof(**big_num));
}

static inline int clock_big_num_index(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch == 'p' || ch == 'P')
        return 10;
    else if (ch == 'm' || ch == 'M')
        return 11;
    else if (ch == 'a' || ch == 'A')
        return 12;
    else if (ch == ':')
        return 13;
    else if (ch == ' ')
        return 14;
    return 0;
}

static void write_clock_str (WINDOW *win, int y, int x, const char *str)
{
    int ch, i, j;

    for (ch = 0; str[ch]; ch++)
        for (i = 0; i < 3; i++)
            for (j = 0; j < 5; j++)
            mvwaddch(win, y + j, x + ch * 4 + i, big_num[clock_big_num_index(str[ch])][j][i]? ACS_BLOCK: ' ');
}

static void clock_update (struct nclyr_win *win)
{
    struct clock_win *clock = container_of(win, struct clock_win, super_win);
    char buf[100];
    time_t tim;
    int win_width, win_height;
    int height, width;
    WINDOW *curwin = win->win;

    getmaxyx(curwin, win_height, win_width);

    tim = time(NULL);

    strftime(buf, sizeof(buf), "%l:%M:%S %p", localtime(&tim));

    if (strcmp(buf, clock->last_buf) != 0) {
        free(clock->last_buf);
        clock->last_buf = strdup(buf);

        clock->color = (clock->color + 1) % 8;
    }

    clock->y += clock->ydir;
    clock->x += clock->xdir;

    clock_strlen(clock->last_buf, &width, &height);

    if (clock->y < 0) {
        clock->y += 2;
        if (clock->y < 0)
            clock->y = 0;
        clock->ydir = -clock->ydir;
    } else if (clock->y + height >= win_height) {
        clock->y -= 2;
        if (clock->y + height >= win_height)
            clock->y = win_height - height;
        clock->ydir = -clock->ydir;
    }

    if (clock->x < 0) {
        clock->x += 2;
        if (clock->x < 0)
            clock->x = 0;
        clock->xdir = -clock->xdir;
    } else if (clock->x + width >= win_width) {
        clock->x -= 2;
        if (clock->x + width >= win_width)
            clock->x = win_width - width;
        clock->xdir = -clock->xdir;
    }

    werase(curwin);

    tui_color_fb_set(win->win, clock->color + 1, CONS_COLOR_DEFAULT);
    write_clock_str(curwin, clock->y, clock->x, clock->last_buf);
    tui_color_fb_unset(win->win, clock->color + 1, CONS_COLOR_DEFAULT);
}

static void clock_init (struct nclyr_win *win)
{
    struct clock_win *clock = container_of(win, struct clock_win, super_win);

    clock->last_buf = malloc(1);
    clock->last_buf[0] = '\0';

    clock->ydir = 1;
    clock->xdir = 1;
}

static void clock_clean (struct nclyr_win *win)
{
    struct clock_win *clock = container_of(win, struct clock_win, super_win);

    free(clock->last_buf);
}

struct clock_win clock_window = {
    .super_win = {
        .win_name = "Clock",
        .win = NULL,
        .timeout = 500,
        .updated = 1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            { '\0', NULL, NULL }
        },
        .init = clock_init,
        .clean = clock_clean,
        .switch_to = NULL,
        .update = clock_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = NULL,
    },
};

