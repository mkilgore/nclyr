
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "config.h"
#include "song.h"
#include "player.h"
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "tui_internal.h"
#include "tui_state.h"
#include "tui_color.h"
#include "window.h"
#include "playlist_win.h"
#include "debug.h"

static struct cons_printf_arg args[] = {
    { .id = "title", .type = CONS_ARG_STRING },
    { .id = "artist", .type = CONS_ARG_STRING },
    { .id = "album", .type = CONS_ARG_STRING },
    { .id = "duration", .type = CONS_ARG_TIME },
    { .id = "play-num", .type = CONS_ARG_INT },
    { .id = "playing", .type = CONS_ARG_BOOL },
    { .id = "selected", .type = CONS_ARG_BOOL },
    { .id = "song", .type = CONS_ARG_SONG },
};

static void handle_mouse(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;

    if (ch != KEY_MOUSE || !mevent)
        return ;

    DEBUG_PRINTF("Playlist Mouse event: %d, (%d, %d)\n", mevent->type, mevent->x, mevent->y);

    if (mevent->type == SCROLL_UP) {
        if (play->disp_offset > 0) {
            play->disp_offset--;
            win->updated = 1;
        }
    } else if (mevent->type == SCROLL_DOWN) {
        if (play->disp_offset < tui->state.playlist.song_count - 1) {
            play->disp_offset++;
            win->updated = 1;
        }
    } else if (mevent->type == LEFT_CLICKED) {
        play->selected = play->disp_offset + mevent->y;
        win->updated = 1;
    }
}

static void handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows;

    switch (ch) {
    case KEY_UP:
    case 'k':
        if (play->selected > 0) {
            play->selected--;
            if (play->selected < play->disp_offset)
                play->disp_offset--;
            win->updated = 1;
        }
        break;
    case KEY_DOWN:
    case 'j':
        if (play->selected < tui->state.playlist.song_count - 1) {
            play->selected++;
            rows = getmaxy(win->win);
            if (play->selected > play->disp_offset + rows - 1)
                play->disp_offset++;
            win->updated = 1;
        }
        break;

    case 'd':
        player_remove_song(player_current(), play->selected);
        break;

    case KEY_LEFT:
    case 'J':
    case KEY_NPAGE:
        if (play->disp_offset < tui->state.playlist.song_count - 1) {
            rows = getmaxy(win->win);
            if (rows < ((tui->state.playlist.song_count - 1) - play->disp_offset))
                play->disp_offset += rows;
            else
                play->disp_offset = tui->state.playlist.song_count - 1;
            win->updated = 1;
        }
        break;
    case KEY_RIGHT:
    case 'K':
    case KEY_PPAGE:
        if (play->disp_offset > 0) {
            rows = getmaxy(win->win);
            if (rows < play->disp_offset)
                play->disp_offset -= rows;
            else
                play->disp_offset = 0;
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
    struct cons_str chstr;
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
            args[7].u.song.s = song;

            cons_str_init(&chstr);
            cons_printf(play->printline, &chstr, cols, tui_get_chtype_from_window(curwin), args, ARRAY_SIZE(args));
            waddchstr(curwin, chstr.chstr);
            cons_str_clear(&chstr);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

static void playlist_win_init(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;

    play->printline = cons_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_PLAYLIST, PRINTLINE)->u.str.str, ARRAY_SIZE(args), args);
    play->song_triple = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_PLAYLIST, SONG_TRIPLE)->u.str.str);
    play->song_filename = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_PLAYLIST, SONG_FILENAME)->u.str.str);

    args[7].u.song.triple = play->song_triple;
    args[7].u.song.filename = play->song_filename;
}

static void playlist_win_clear(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);

    cons_printf_compiled_free(play->printline);
    cons_printf_compiled_free(play->song_triple);
    cons_printf_compiled_free(play->song_filename);
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

static struct playlist_win playlist_window_init = {
    .super_win = {
        .win_name = "Playlist",
        .win = NULL,
        .timeout = 400,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            N_KEYPRESS('j', handle_ch, "Select song below"),
            N_KEYPRESS('k', handle_ch, "Select song above"),
            N_KEYPRESS(KEY_UP, handle_ch, "Select song below"),
            N_KEYPRESS(KEY_DOWN, handle_ch, "Select song above"),
            N_KEYPRESS('\n', handle_ch, "Play selected song"),
            N_KEYPRESS('d', handle_ch, "Remove selected song"),
            N_MOUSE(SCROLL_UP, handle_mouse, "Scroll playlist up"),
            N_MOUSE(SCROLL_DOWN, handle_mouse, "Scroll playlist down"),
            N_MOUSE(LEFT_CLICKED, handle_mouse, "Select song"),
            N_END()
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

struct nclyr_win *playlist_win_new(void)
{
    struct playlist_win *win = malloc(sizeof(*win));
    memcpy(win, &playlist_window_init, sizeof(playlist_window_init));
    return &win->super_win;
}

