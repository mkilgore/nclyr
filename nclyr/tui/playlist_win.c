
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "config.h"
#include "song.h"
#include "player.h"
#include "cons_color.h"
#include "tui_internal.h"
#include "tui_chstr.h"
#include "tui_printf.h"
#include "tui_color.h"
#include "tui_window.h"
#include "playlist_win.h"
#include "debug.h"

static struct tui_printf_arg args[] = {
    { .id = "title", .type = TUI_ARG_STRING },
    { .id = "artist", .type = TUI_ARG_STRING },
    { .id = "album", .type = TUI_ARG_STRING },
    { .id = "duration", .type = TUI_ARG_INT },
    { .id = "play-num", .type = TUI_ARG_INT },
    { .id = "playing", .type = TUI_ARG_BOOL },
    { .id = "selected", .type = TUI_ARG_BOOL },
};

static void handle_ch(struct nclyr_win *win, int ch)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows;

    switch (ch) {
    case 'k':
        if (play->selected > 0) {
            play->selected--;
            if (play->selected < play->disp_offset)
                play->disp_offset--;
            win->updated = 1;
        }
        break;
    case 'j':
        if (play->selected < tui->state.playlist.song_count - 1) {
            play->selected++;
            rows = getmaxy(win->win);
            if (play->selected > play->disp_offset + rows - 1)
                play->disp_offset++;
            win->updated = 1;
        }
        break;
    case '\n':
        player_change_song(player_current(), play->selected);
        break;
    }
}

static void playlist_win_update(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    struct chstr chstr;
    int rows, cols, i;
    WINDOW *curwin = win->win;

    win->updated = 0;

    werase(curwin);

    getmaxyx(curwin, rows, cols);

    for (i = 0; i < rows; i++) {
        if (tui->state.playlist.song_count > i + play->disp_offset) {
            struct song_info *song = tui->state.playlist.songs[i + play->disp_offset];
            int is_sel = 0, is_play = 0;

            if (i + play->disp_offset == play->selected)
                is_sel = 1;

            if (i + play->disp_offset == tui->state.song_pos)
                is_play = 1;

            wmove(curwin, i, 0);

            args[0].u.str_val = song->tag.title;
            args[1].u.str_val = song->tag.artist;
            args[2].u.str_val = song->tag.album;
            args[3].u.int_val = song->duration;
            args[4].u.int_val = i + play->disp_offset + 1;
            args[5].u.bool_val = is_play;
            args[6].u.bool_val = is_sel;

            chstr_init(&chstr);
            tui_printf(play->printline, &chstr, cols, tui_get_chtype_from_window(curwin), args, ARRAY_SIZE(args));
            waddchstr(curwin, chstr.chstr);
            chstr_clear(&chstr);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

static void playlist_win_init(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    play->printline = tui_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_PLAYLIST, PRINTLINE)->u.str.str, ARRAY_SIZE(args), args);
}

static void playlist_win_clear(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);

    tui_printf_compile_free(play->printline);
}

static void playlist_win_new_player_notif(struct nclyr_win *win, enum player_notif_type notif, struct player_state_full *state)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    if (notif == PLAYER_SONG_POS || notif == PLAYER_PLAYLIST)
        win->updated = 1;

    if (notif == PLAYER_PLAYLIST)
        play->selected = 0;

    return ;
}

struct playlist_win playlist_window = {
    .super_win = {
        .win_name = "Playlist",
        .win = NULL,
        .timeout = 400,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            { 'j', handle_ch, "Select song above" },
            { 'k', handle_ch, "Select song below" },
            { '\n', handle_ch, "Play selected song" },
            { '\0', NULL, NULL }
        },
        .init = playlist_win_init,
        .clean = playlist_win_clear,
        .switch_to = NULL,
        .update = playlist_win_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = playlist_win_new_player_notif,
    },
    .selected = 0,
    .disp_offset = 0
};

