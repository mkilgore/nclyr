
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
#include "lyrics_win.h"

static void lyrics_update (struct nclyr_win *win)
{
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);
    WINDOW *curwin = win->win;
    werase(curwin);

    if (lyrics->lines == 0) {
        mvwprintw(curwin, 0, 0, "Lyrics are not yet available.");
    } else {
        int i, rows, cols;

        getmaxyx(curwin, rows, cols);
        for (i = 0; i < rows && i + lyrics->disp_offset < lyrics->lines; i++)
            mvwprintw(curwin, i, 0, "%-*s", cols, lyrics->cur_lyrics[i + lyrics->disp_offset]);
    }

    wrefresh(curwin);
}

static inline void free_lyrics_lines (struct lyrics_win *lyrics)
{
    int i;
    for (i = 0; i < lyrics->lines; i++)
        free(lyrics->cur_lyrics[i]);

    free(lyrics->cur_lyrics);

    lyrics->lines = 0;
    lyrics->disp_offset = 0;
    lyrics->cur_lyrics = NULL;
}

static void lyrics_init (struct nclyr_win *win, int y, int x, int rows, int cols)
{
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);

    win->win = newwin(rows, cols, y, x);

    lyrics->lines = 0;
    lyrics->disp_offset = 0;
    lyrics->cur_lyrics = NULL;
}

static void lyrics_clean (struct nclyr_win *win)
{
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);

    delwin(win->win);
    free_lyrics_lines(lyrics);
}

static void lyrics_resize (struct nclyr_win *win, int y, int x, int rows, int cols)
{
    delwin(win->win);
    win->win = newwin(rows, cols, y, x);
    touchwin(win->win);
}

static void lyrics_clear_song_data (struct nclyr_win *win)
{
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);

    free_lyrics_lines(lyrics);
}

static void lyrics_new_song_data (struct nclyr_win *win, const struct lyr_thread_notify *song_notif)
{
    char *start, *ptr;
    size_t line_count;
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);

    if (song_notif->type != LYR_LYRICS)
        return ;

    free_lyrics_lines(lyrics);

    line_count = 0;
    for (ptr = song_notif->u.lyrics; *ptr; ptr++)
        if (*ptr == '\n')
            line_count++;

    lyrics->lines = line_count;
    lyrics->disp_offset = 0;
    lyrics->cur_lyrics = malloc(line_count * sizeof(char *));

    line_count = 0;
    for (start = ptr = song_notif->u.lyrics; *ptr; ptr++) {
        if (*ptr == '\n') {
            lyrics->cur_lyrics[line_count] = malloc(ptr - start + 1);
            memset(lyrics->cur_lyrics[line_count], 0, ptr - start + 1);
            if (ptr - start > 0)
                memcpy(lyrics->cur_lyrics[line_count], start, ptr - start);
            lyrics->cur_lyrics[line_count][ptr - start] = '\0';
            line_count++;
            start = ptr + 1;
        }
    }

}

static void handle_lyrics_keypress(struct nclyr_win *win, int ch)
{
    struct lyrics_win *lyrics = container_of(win, struct lyrics_win, super_win);
    switch (ch) {
    case 'j':
        if (lyrics->disp_offset < lyrics->lines - 1)
            lyrics->disp_offset++;
        break;
    case 'k':
        if (lyrics->disp_offset > 0)
            lyrics->disp_offset--;
        break;
    }
}

static void lyrics_switch_to (struct nclyr_win *win)
{
    return ;
}

static void lyrics_new_player_notif (struct nclyr_win *win, const struct player_notification *notif)
{
    return ;
}

struct lyrics_win lyrics_window = {
    .super_win = {
        .win_name = "Lyrics",
        .win = NULL,
        .timeout = -1,
        .lyr_types = (const enum lyr_data_type[]) { LYR_LYRICS, -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            { 'j', handle_lyrics_keypress, "Scroll lyrics down."},
            { 'k', handle_lyrics_keypress, "Scroll lyrics up." },
            { '\0', NULL, NULL }
        },
        .init = lyrics_init,
        .clean = lyrics_clean,
        .switch_to = lyrics_switch_to,
        .update = lyrics_update,
        .resize = lyrics_resize,
        .clear_song_data = lyrics_clear_song_data,
        .new_song_data = lyrics_new_song_data,
        .new_player_notif = lyrics_new_player_notif,
    },
    .lines = 0,
    .disp_offset = 0,
    .cur_lyrics = NULL,
};

