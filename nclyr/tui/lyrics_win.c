
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "player.h"
#include "tui/window.h"
#include "line_win.h"
#include "window_center.h"
#include "lyrics_win.h"

static void lyrics_update (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    if (line->line_count == 0) {
        werase(win->win);
        win_center_str(win->win, "Lyrics not available");
        wrefresh(win->win);
    } else {
        line_update(win);
    }
}

static void lyrics_new_song_data (struct nclyr_win *win, const struct lyr_thread_notify *song_notif)
{
    char *start, *ptr;
    size_t line_count;
    struct line_win *line = container_of(win, struct line_win, super_win);

    if (song_notif->type != LYR_LYRICS)
        return ;

    line_free_lines(line);

    line_count = 0;
    for (ptr = song_notif->u.lyrics; *ptr; ptr++)
        if (*ptr == '\n')
            line_count++;

    line->line_count = line_count;
    line->disp_offset = 0;
    line->lines = malloc(line_count * sizeof(char *));

    line_count = 0;
    for (start = ptr = song_notif->u.lyrics; *ptr; ptr++) {
        if (*ptr == '\n') {
            line->lines[line_count] = malloc(ptr - start + 1);
            memset(line->lines[line_count], 0, ptr - start + 1);
            if (ptr - start > 0)
                memcpy(line->lines[line_count], start, ptr - start);
            line->lines[line_count][ptr - start] = '\0';
            line_count++;
            start = ptr + 1;
        }
    }
}

void lyrics_clear_song_data (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    line_free_lines(line);
}

void lyrics_new_player_notif(struct nclyr_win *win, const struct player_notification *notif)
{
    return ;
}

static void lyrics_handle_keypress(struct nclyr_win *win, int ch)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    switch (ch) {
    case 'c':
        line->center = !line->center;
        break;
    }
}

struct line_win lyrics_window = {
    .super_win = {
        .win_name = "Lyrics",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_LYRICS, -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            LINE_KEYPRESSES,
            { 'c', lyrics_handle_keypress, "Toggle line centering" },
            { '\0', NULL, NULL }
        },
        .init = line_init,
        .clean = line_clean,
        .switch_to = line_switch_to,
        .update = lyrics_update,
        .resize = line_resize,
        .clear_song_data = lyrics_clear_song_data,
        .new_song_data = lyrics_new_song_data,
        .new_player_notif = lyrics_new_player_notif,
    },
    .line_count = 0,
    .disp_offset = 0,
    .lines = NULL,
    .center = 1
};

