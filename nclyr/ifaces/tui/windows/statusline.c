
#include "common.h"

#include <ncurses.h>

#include "a_sprintf.h"
#include "config.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "tui_internal.h"
#include "tui_state.h"
#include "player.h"
#include "statusline.h"
#include "debug.h"

static struct cons_printf_arg args[] = {
    { .id = "title", .type = CONS_ARG_STRING },
    { .id = "artist", .type = CONS_ARG_STRING },
    { .id = "album", .type = CONS_ARG_STRING },
    { .id = "duration", .type = CONS_ARG_TIME },
    { .id = "position", .type = CONS_ARG_TIME },
    { .id = "paused", .type = CONS_ARG_BOOL },
    { .id = "volume", .type = CONS_ARG_INT },
    { .id = "song", .type = CONS_ARG_SONG },
};

void statusline_update(struct statusline *status)
{
    struct tui_iface *tui = status->tui;
    struct song_info *song = tui->state.song;
    struct cons_str chstr;
    status->updated = 0;

    if (tui->state.is_up) {
        int cols, bar_len, x;
        werase(status->win);

        cols = getmaxx(status->win);

        wmove(status->win, 0, 0);
        if (tui->state.song) {
            args[0].u.str_val = song->tag.title;
            args[1].u.str_val = song->tag.artist;
            args[2].u.str_val = song->tag.album;
            args[3].u.time_val = song->duration;
            args[4].u.time_val = tui->state.seek_pos;
            args[5].u.bool_val = tui->state.state == PLAYER_PAUSED;
            args[6].u.int_val = tui->state.volume;
            args[7].u.song.s = song;

            cons_str_init(&chstr);
            cons_printf(status->song_name, &chstr, cols, tui_get_chtype_from_window(status->win), args, ARRAY_SIZE(args));
            waddchstr(status->win, chstr.chstr);
            cons_str_clear(&chstr);
        } else {
            wprintw(status->win, "No song playing");
        }

        if (tui->state.song) {
            bar_len = (int)((float)cols * ((float)tui->state.seek_pos / (float)tui->state.song->duration));

            DEBUG_PRINTF("Bar_len: %d\n", bar_len);
            /*
            mvwprintw(status->win, 0, cols - 13, "[%02d:%02d/%02d:%02d]",
                    tui->state.seek_pos / 60, tui->state.seek_pos % 60,
                    tui->state.song->duration / 60, tui->state.song->duration % 60); */

            mvwprintw(status->win, 1, 0, "%.*s", cols, "");

            for (x = 0; x < bar_len; x++)
                mvwaddch(status->win, 1, x, '=');

            mvwaddch(status->win, 1, bar_len, '>');
        }

    } else {
        struct player *player = player_current();
        werase(status->win);
        mvwprintw(status->win, 0, 0, "%s is not currently running", player->name);
    }
}

void statusline_init(struct statusline *status, int cols)
{
    struct tui_iface *tui = status->tui;
    status->win = newwin(2, cols, 0, 0);
    status->updated = 1;

    status->song_name = cons_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_STATUSLINE, SONG)->u.str.str, ARRAY_SIZE(args), args);
    status->song_triple = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_STATUSLINE, SONG_TRIPLE)->u.str.str);
    status->song_filename = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_STATUSLINE, SONG_FILENAME)->u.str.str);

    args[7].u.song.triple = status->song_triple;
    args[7].u.song.filename = status->song_filename;
}

void statusline_clean(struct statusline *status)
{
    cons_printf_compiled_free(status->song_name);
    delwin(status->win);
}

void statusline_resize(struct statusline *status, int cols)
{
    wresize(status->win, 2, cols);
    status->updated = 1;
}

void statusline_player_notif(struct statusline *status, enum player_notif_type notif, struct player_state_full *state)
{
    if (notif == PLAYER_SEEK
            || notif == PLAYER_SONG
            || notif == PLAYER_NO_SONG
            || notif == PLAYER_IS_DOWN
            || notif == PLAYER_IS_UP
            || notif == PLAYER_VOLUME
            || notif == PLAYER_STATE)
        status->updated = 1;
}

struct statusline statusline = {
    .win = NULL,
    .init = statusline_init,
    .clean = statusline_clean,
    .resize = statusline_resize,
    .update = statusline_update,
    .player_notif = statusline_player_notif
};

