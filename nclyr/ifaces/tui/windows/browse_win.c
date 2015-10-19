
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

struct browse_win {
    struct selectable_win sel;

    struct directory_entry *entries;

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

static void browse_win_get_line(struct selectable_win *win, int line, int is_sel, struct cons_str *chstr)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel);
    struct directory_entry *entry;
    int cols;

    cols = getmaxx(win->super_win.win);

    entry = brow->entries + line;
    switch (entry->type) {
    case ENTRY_TYPE_DIR:
        dir_args[0].u.str_val = entry->name;
        dir_args[1].u.bool_val = is_sel;
        cons_printf(brow->directory, chstr, cols, tui_get_chtype_from_window(win->super_win.win), dir_args, ARRAY_SIZE(dir_args));
        break;

    case ENTRY_TYPE_SONG:
        song_args[0].u.str_val = entry->song->tag.title;
        song_args[1].u.str_val = entry->song->tag.artist;
        song_args[2].u.str_val = entry->song->tag.album;
        song_args[3].u.time_val = entry->song->duration;
        song_args[4].u.str_val = entry->song->name;
        song_args[5].u.bool_val = is_sel;
        song_args[6].u.song.s = entry->song;
        cons_printf(brow->song, chstr, cols, tui_get_chtype_from_window(win->super_win.win), song_args, ARRAY_SIZE(song_args));
        break;
    }
}

static void browse_win_clear_entries(struct browse_win *brow)
{
    int i;
    for (i = 0; i < brow->sel.total_lines; i++)
        directory_entry_clear(brow->entries + i);

    free(brow->entries);
}

static void browse_win_init(struct nclyr_win *win)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    brow->directory = cons_printf_compile("${if;selected:true}${reverse}${endif} [${name}]${right_align} ", ARRAY_SIZE(dir_args), dir_args);
    brow->song = cons_printf_compile("${if;selected:true}${reverse}${endif} ${song}${right_align}${duration;pad:true;seconds:true;minutes:true}", ARRAY_SIZE(song_args), song_args);
    brow->song_filename = cons_printf_compile_song("${filename}");
    brow->song_triple = cons_printf_compile_song("${color;f:cyan} ${title} ${color;f:default}"
                                                      ">>${color;f:red} ${artist} ${color;f:default}"
                                                      ">>${color;f:green} ${album} ");

    song_args[6].u.song.triple = brow->song_triple;
    song_args[6].u.song.filename = brow->song_filename;

    brow->sel.super_win.win_name = strdup("Browse");
}

static void browse_win_clear(struct nclyr_win *win)
{
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    cons_printf_compiled_free(brow->directory);
    cons_printf_compiled_free(brow->song);
    cons_printf_compiled_free(brow->song_triple);
    cons_printf_compiled_free(brow->song_filename);

    browse_win_clear_entries(brow);

    free(win->win_name);
}

static void browse_win_new_player_notif(struct nclyr_win *win, enum player_notif_type notif, struct player_state_full *state)
{
    struct directory_entry *entry;
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);
    int i;

    if (notif != PLAYER_DIRECTORY)
        return ;

    browse_win_clear_entries(brow);

    win->updated = 1;
    sel->selected = 0;
    sel->disp_offset = 0;
    sel->total_lines = state->cwd.entry_count;

    brow->entries = malloc(sizeof(*brow->entries) * sel->total_lines);

    i = 0;
    list_foreach_entry(&state->cwd.entries, entry, struct directory_entry, dir_entry) {
        directory_entry_copy(brow->entries + i, entry);
        i++;
    }

    win->updated = 1;

    a_sprintf(&win->win_name, "Browse - %s", state->cwd.name);

    return ;
}

static void browse_win_handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    struct browse_win *brow = container_of(win, struct browse_win, sel.super_win);

    switch (ch) {
    case '\n':
        if (brow->entries[sel->selected].type == ENTRY_TYPE_DIR) {
            player_change_working_directory(player_current(), strdup(brow->entries[sel->selected].name));
            player_get_working_directory(player_current());
        } else {
            player_add_song(player_current(), strdup(brow->entries[sel->selected].song->name));
        }
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
                N_KEYPRESS('\n', browse_win_handle_ch, "Change Directory\\Add Song"),
                N_END()
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
    },
};

struct nclyr_win *browse_win_new(void)
{
    struct browse_win *win = malloc(sizeof(*win));
    memcpy(win, &browse_window_init, sizeof(browse_window_init));
    return &win->sel.super_win;
}

