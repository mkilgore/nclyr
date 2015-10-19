
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "player.h"
#include "tui_internal.h"
#include "window.h"
#include "line_win.h"
#include "lyrics_win.h"
#include "window_center.h"
#include "artist_win.h"
#include "debug.h"

struct artist_win {
    struct line_win line;

    int being_looked_up;
};

static void artist_update (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    struct artist_win *art = container_of(line, struct artist_win, line);

    if (line->line_count == 0) {
        werase(win->win);
        if (!art->being_looked_up)
            win_center_str(win->win, "Artist bio not available");
        else
            win_center_str(win->win, "Artist bio being found...");
    } else {
        line_update(win);
    }
}

static void artist_new_song_data (struct nclyr_win *win, const struct lyr_thread_notify *song_notif)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    struct artist_win *art = container_of(line, struct artist_win, line);

    DEBUG_PRINTF("Got artist song_notif\n");

    if (song_notif->type != LYR_ARTIST_BIO)
        return ;

    line_free_lines(line);
    art->being_looked_up = 0;
    win->updated = 1;

    if (!song_notif->was_recieved)
        return ;

    line->line_count = 1;
    line->disp_offset = 0;
    line->lines = malloc(line->line_count * sizeof(char *));
    line->lines[0] = strdup(song_notif->u.bio);
}

void artist_clear_song_data (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    struct artist_win *art = container_of(line, struct artist_win, line);

    line_free_lines(line);
    art->being_looked_up = 0;
    win->updated = 1;
}

void artist_lookup_started(struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    struct artist_win *art = container_of(line, struct artist_win, line);

    art->being_looked_up = 1;
    win->updated = 1;
}

static struct artist_win artist_window_init = {
    .line = {
        .super_win = {
            .win_name = "Artist bio",
            .win = NULL,
            .timeout = -1,
            .lyr_types = (const enum lyr_data_type[]) { LYR_ARTIST_BIO, LYR_DATA_TYPE_COUNT },
            .keypresses = (const struct nclyr_keypress[]) {
                LINE_KEYPRESSES(),
                N_END()
            },
            .init = NULL,
            .clean = line_clean,
            .switch_to = NULL,
            .update = artist_update,
            .resize = NULL,
            .clear_song_data = artist_clear_song_data,
            .new_song_data = artist_new_song_data,
            .new_player_notif = NULL,
            .lookup_started = artist_lookup_started,
        },
        .line_count = 0,
        .disp_offset = 0,
        .lines = NULL,
    },
    .being_looked_up = 0,
};

struct nclyr_win *artist_win_new(void)
{
    struct artist_win *win = malloc(sizeof(*win));
    memcpy(win, &artist_window_init, sizeof(artist_window_init));
    return &win->line.super_win;
}

