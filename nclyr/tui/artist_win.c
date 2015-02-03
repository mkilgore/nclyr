
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
#include "lyrics_win.h"
#include "window_center.h"
#include "artist_win.h"
#include "debug.h"

static void artist_update (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    if (line->line_count == 0) {
        werase(win->win);
        win_center_str(win->win, "Artist bio not available");
    } else {
        line_update(win);
    }
}

static void artist_new_song_data (struct nclyr_win *win, const struct lyr_thread_notify *song_notif)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    DEBUG_PRINTF("Got artist song_notif\n");

    if (song_notif->type != LYR_ARTIST_BIO)
        return ;

    line_free_lines(line);

    line->line_count = 1;
    line->disp_offset = 0;
    line->lines = malloc(line->line_count * sizeof(char *));
    line->lines[0] = strdup(song_notif->u.bio);

    win->updated = 1;

    /*
    line_count = 0;
    for (ptr = song_notif->u.bio; *ptr; ptr++)
        if (*ptr == '\n')
            line_count++;

    line->line_count = line_count;
    line->disp_offset = 0;
    line->lines = malloc(line_count * sizeof(char *));

    line_count = 0;
    for (start = ptr = song_notif->u.bio; *ptr; ptr++) {
        if (*ptr == '\n') {
            line->lines[line_count] = malloc(ptr - start + 1);
            memset(line->lines[line_count], 0, ptr - start + 1);
            if (ptr - start > 0)
                memcpy(line->lines[line_count], start, ptr - start);
            line->lines[line_count][ptr - start] = '\0';
            line_count++;
            start = ptr + 1;
        }
    } */
}

void artist_clear_song_data (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    line_free_lines(line);
    win->updated = 1;
}

struct line_win artist_window = {
    .super_win = {
        .win_name = "Artist bio",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_ARTIST_BIO, -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            LINE_KEYPRESSES,
            { '\0', NULL, NULL }
        },
        .init = NULL,
        .clean = line_clean,
        .switch_to = NULL,
        .update = artist_update,
        .resize = NULL,
        .clear_song_data = artist_clear_song_data,
        .new_song_data = artist_new_song_data,
        .new_player_notif = NULL,
    },
    .line_count = 0,
    .disp_offset = 0,
    .lines = NULL,
};

