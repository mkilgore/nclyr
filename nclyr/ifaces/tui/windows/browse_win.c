
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "config.h"
#include "song.h"
#include "directory.h"
#include "player.h"
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "tui_internal.h"
#include "tui_state.h"
#include "tui_color.h"
#include "window.h"
#include "browse_win.h"
#include "selectable_win.h"
#include "debug.h"

#define MAX_DIRECTORY_HISTORY 20

struct history_pos {
    int selected;
    int disp_offset;
};

struct browse_win {
    struct selectable_win sel;

    /* Keeps a history of how many levels deep, and the past selections made.
     * Note we count the levels past 'MAX_DIRECTORY_HISTORY', but only store
     * MAX_DIRECTORY_HISTORY levels. Levels past that won't have their
     * selection stored.
     *
     * This feature allows you to traverse into a directory, and then exit that
     * directory and not have to scroll back to where you were. */
    int directory_level;
    struct history_pos directory_history[MAX_DIRECTORY_HISTORY];

    int next_sel;
    int next_disp_off;

    int waiting_for_directory;

    cons_printf_compiled *directory;
    cons_printf_compiled *song;
    cons_printf_compiled *song_triple;
    cons_printf_compiled *song_filename;
};

static struct cons_printf_arg dir_args[] = {
    { .id = "name", .type = CONS_ARG_STRING },
    { .id = "selected", .type = CONS_ARG_BOOL },
};

static struct cons_printf_arg song_args[] = {
    { .id = "title", .type = CONS_ARG_STRING },
    { .id = "artist", .type = CONS_ARG_STRING },
    { .id = "album", .type = CONS_ARG_STRING },
    { .id = "duration", .type = CONS_ARG_TIME },
    { .id = "name", .type = CONS_ARG_STRING },
    { .id = "selected", .type = CONS_ARG_BOOL },
    { .id = "song", .type = CONS_ARG_SONG },
};

static void browse_win_get_line(struct selectable_win *win, int line, int width, struct cons_str *chstr)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel);
    struct directory_entry *entry;
    struct tui_iface *tui = win->super_win.tui;

    entry = tui->state.cwd.entries + line;

    switch (entry->type) {
    case ENTRY_TYPE_DIR:
        dir_args[0].u.str_val = entry->name;
        dir_args[1].u.bool_val = line == win->selected;
        cons_printf(brow->directory, chstr, width, tui_get_chtype_from_window(win->super_win.win), dir_args, ARRAY_SIZE(dir_args));
        break;

    case ENTRY_TYPE_SONG:
        song_args[0].u.str_val = entry->song->tag.title;
        song_args[1].u.str_val = entry->song->tag.artist;
        song_args[2].u.str_val = entry->song->tag.album;
        song_args[3].u.time_val = entry->song->duration;
        song_args[4].u.str_val = entry->song->name;
        song_args[5].u.bool_val = line == win->selected;
        song_args[6].u.song.s = entry->song;
        cons_printf(brow->song, chstr, width, tui_get_chtype_from_window(win->super_win.win), song_args, ARRAY_SIZE(song_args));
        break;
    }
}

static void browse_win_line_selected(struct selectable_win *sel)
{
    struct browse_win *brow = container_of(sel, struct browse_win, sel);
    struct tui_iface *tui = sel->super_win.tui;

    if (tui->state.cwd.entries[sel->selected].type == ENTRY_TYPE_DIR) {
        player_change_working_directory(player_current(), strdup(tui->state.cwd.entries[sel->selected].name));
        player_get_working_directory(player_current());

        if (strcmp(tui->state.cwd.entries[sel->selected].name, "..") != 0) {
            brow->directory_history[brow->directory_level].selected = sel->selected;
            brow->directory_history[brow->directory_level].disp_offset = sel->disp_offset;
            brow->directory_level++;
            brow->next_sel = 0;
            brow->next_disp_off = 0;
        } else {
            brow->directory_level--;
            brow->next_sel = brow->directory_history[brow->directory_level].selected;
            brow->next_disp_off = brow->directory_history[brow->directory_level].disp_offset;
        }

        brow->waiting_for_directory = 1;
    } else {
        player_add_song(player_current(), strdup(tui->state.cwd.entries[sel->selected].song->name));
    }
}

static void browse_win_init(struct nclyr_win *win)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);
    struct tui_iface *tui = win->tui;

    brow->directory = cons_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_BROWSE, DIRECTORY)->u.str.str, ARRAY_SIZE(dir_args), dir_args);
    brow->song = cons_printf_compile(CONFIG_GET(tui->cfg, TUI_CONFIG_BROWSE, SONG)->u.str.str, ARRAY_SIZE(song_args), song_args);
    brow->song_triple = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_BROWSE, SONG_TRIPLE)->u.str.str);
    brow->song_filename = cons_printf_compile_song(CONFIG_GET(tui->cfg, TUI_CONFIG_BROWSE, SONG_FILENAME)->u.str.str);

    song_args[6].u.song.triple = brow->song_triple;
    song_args[6].u.song.filename = brow->song_filename;

    brow->next_sel = 0;

    brow->sel.super_win.win_name = strdup("Browse");
}

static void browse_win_clear(struct nclyr_win *win)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    cons_printf_compiled_free(brow->directory);
    cons_printf_compiled_free(brow->song);
    cons_printf_compiled_free(brow->song_triple);
    cons_printf_compiled_free(brow->song_filename);

    free(win->win_name);
}

static void browse_win_new_player_notif(struct nclyr_win *win, enum player_notif_type notif, struct player_state_full *state)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    if (notif != PLAYER_DIRECTORY)
        return ;

    win->updated = 1;
    sel->selected = brow->next_sel;
    sel->disp_offset = brow->next_disp_off;
    sel->total_lines = state->cwd.entry_count;
    brow->waiting_for_directory = 0;

    a_sprintf(&win->win_name, "Browse - %s", state->cwd.name);

    return ;
}

static void browse_win_handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    /* We don't handle any keypresses while we're waiting for the player to
     * give us a directory. If we did, then we would risk sending multiple 'get
     * directory' commands, along other things */
    if (brow->waiting_for_directory)
        return ;

    switch (ch) {
    case '\n':
        browse_win_line_selected(sel);
        break;
    }
}

static struct browse_win browse_window_init = {
    .sel = {
        .super_win = {
            .win_name = NULL, /* We change this dynamically */
            .win = NULL,
            .timeout = -1,
            .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
            .keypresses = (const struct nclyr_keypress[]) {
                SELECTABLE_KEYPRESSES(),
                NCLYR_KEYPRESS('\n', browse_win_handle_ch, "Change Directory\\Add Song"),
                NCLYR_END()
            },
            .init = browse_win_init,
            .clean = browse_win_clear,
            .switch_to = NULL,
            .update = selectable_win_update,
            .resize = NULL,
            .clear_song_data = NULL,
            .new_song_data = NULL,
            .new_player_notif = browse_win_new_player_notif,
        },
        .selected = 0,
        .disp_offset = 0,
        .total_lines = 0,
        .get_line = browse_win_get_line,
        .line_selected = browse_win_line_selected,
    },
};

struct nclyr_win *browse_win_new(void)
{
    struct browse_win *win = malloc(sizeof(*win));
    memcpy(win, &browse_window_init, sizeof(browse_window_init));
    return &win->sel.super_win;
}

