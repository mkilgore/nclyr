
#include "common.h"

#include <ncurses.h>

#include "a_sprintf.h"
#include "config.h"
#include "tui_internal.h"
#include "tui_chstr.h"
#include "tui_printf.h"
#include "player.h"
#include "statusline.h"
#include "debug.h"

static struct tui_printf_arg args[4] = {
    { .id = "title", .type = TUI_ARG_STRING },
    { .id = "artist", .type = TUI_ARG_STRING },
    { .id = "album", .type = TUI_ARG_STRING },
    { .id = "duration", .type = TUI_ARG_INT },
};

void statusline_update(struct statusline *status)
{
    struct tui_iface *tui = status->tui;
    struct song_info *song = tui->state.song;
    struct chstr chstr;
    status->updated = 0;

    if (tui->state.is_up) {
        int cols, bar_len, x;
        werase(status->win);

        wmove(status->win, 0, 0);
        if (tui->state.song) {
            args[0].u.str_val = song->tag.title;
            args[1].u.str_val = song->tag.artist;
            args[2].u.str_val = song->tag.album;
            args[3].u.int_val = song->duration;
            chstr_init(&chstr);
            tui_printf(&chstr, tui_get_chtype_from_window(status->win), 0, status->song_name, ARRAY_SIZE(args), args);
            waddchstr(status->win, chstr.chstr);
            chstr_clear(&chstr);
        } else {
            wprintw(status->win, "No song playing");
        }

        cols = getmaxx(status->win);

        if (tui->state.song) {
            bar_len = (int)((float)cols * ((float)tui->state.seek_pos / (float)tui->state.song->duration));

            DEBUG_PRINTF("Bar_len: %d\n", bar_len);
            mvwprintw(status->win, 0, cols - 13, "[%02d:%02d/%02d:%02d]",
                    tui->state.seek_pos / 60, tui->state.seek_pos % 60,
                    tui->state.song->duration / 60, tui->state.song->duration % 60);

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

    status->song_name = tui_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_STATUSLINE, SONG)->u.str.str, ARRAY_SIZE(args), args);
}

void statusline_clean(struct statusline *status)
{
    tui_printf_compile_free(status->song_name);
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
            || notif == PLAYER_IS_UP)
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

