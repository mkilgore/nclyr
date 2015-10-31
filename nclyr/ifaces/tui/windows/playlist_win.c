
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
#include "selectable_win.h"
#include "playlist_win.h"
#include "debug.h"

struct playlist_win {
    struct selectable_win super_win;

    cons_printf_compiled *printline;
    cons_printf_compiled *song_triple;
    cons_printf_compiled *song_filename;

    int waiting_for_song_change;
    int waiting_for_playlist;
};

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

static void playlist_win_line_selected(struct selectable_win *sel)
{
    struct playlist_win *play = container_of(sel, struct playlist_win, super_win);
    struct tui_iface *tui = sel->super_win.tui;

    if (sel->selected == tui->state.song_pos)
        return ;

    if (play->waiting_for_song_change)
        return ;

    play->waiting_for_song_change = 1;

    player_change_song(player_current(), sel->selected);
}

static void playlist_win_handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    struct playlist_win *play = container_of(win, struct playlist_win, super_win.super_win);

    switch (ch) {
    case '\n':
        playlist_win_line_selected(sel);
        break;

    case 'd':
        if (play->waiting_for_playlist)
            break;

        play->waiting_for_playlist = 1;

        player_remove_song(player_current(), sel->selected);
        if (sel->selected > 0)
            sel->selected--;

        if (sel->selected < sel->disp_offset)
            sel->disp_offset--;
        break;
    }
}

static void playlist_win_get_line(struct selectable_win *win, int line, int width, struct cons_str *chstr)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->super_win.tui;
    struct song_info *song = tui->state.playlist.songs[line];
    int is_play = 0;

    if (line == tui->state.song_pos)
        is_play = 1;

    args[0].u.str_val = song->tag.title;
    args[1].u.str_val = song->tag.artist;
    args[2].u.str_val = song->tag.album;
    args[3].u.int_val = song->duration;
    args[4].u.int_val = line + 1; /* Numbers start at 1, not zero */
    args[5].u.bool_val = is_play;
    args[6].u.bool_val = line == win->selected;
    args[7].u.song.s = song;

    cons_printf(play->printline, chstr, width, tui_get_chtype_from_window(win->super_win.win), args, ARRAY_SIZE(args));
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
    struct playlist_win *play = container_of(win, struct playlist_win, super_win.super_win);
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);

    if (notif == PLAYER_SONG_POS) {
        win->updated = 1;
        play->waiting_for_song_change = 0;
    }

    if (notif == PLAYER_PLAYLIST) {
        win->updated = 1;
        sel->total_lines = state->playlist.song_count;
        if (sel->selected >= sel->total_lines)
            sel->selected = sel->total_lines - 1;
        play->waiting_for_playlist = 0;
    }

    return ;
}

static struct playlist_win playlist_window_init = {
    .super_win = {
        .super_win = {
            .win_name = "Playlist",
            .win = NULL,
            .timeout = -1,
            .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
            .keypresses = (const struct nclyr_keypress[]) {
                SELECTABLE_KEYPRESSES(),
                NCLYR_KEYPRESS('\n', playlist_win_handle_ch, "Play selected song"),
                NCLYR_KEYPRESS('d', playlist_win_handle_ch, "Remove selected song"),
                NCLYR_END()
            },
            .init = playlist_win_init,
            .clean = playlist_win_clear,
            .switch_to = NULL,
            .update = selectable_win_update,
            .resize = NULL,
            .clear_song_data = NULL,
            .new_song_data = NULL,
            .new_player_notif = playlist_win_new_player_notif,
        },
        .selected = 0,
        .disp_offset = 0,
        .total_lines = 0,
        .get_line = playlist_win_get_line,
        .line_selected = playlist_win_line_selected,
    }
};

struct nclyr_win *playlist_win_new(void)
{
    struct playlist_win *win = malloc(sizeof(*win));
    memcpy(win, &playlist_window_init, sizeof(playlist_window_init));
    return &win->super_win.super_win;
}

