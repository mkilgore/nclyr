
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "cons/printf.h"
#include "tui_internal.h"
#include "tui_state.h"
#include "tui_color.h"
#include "window.h"
#include "manager_win.h"
#include "debug.h"

static struct cons_printf_arg args[] = {
    { .id = "name", .type = CONS_ARG_STRING },
    { .id = "win_num", .type = CONS_ARG_INT },
    { .id = "selected", .type = CONS_ARG_BOOL },
};

static void manager_win_init(struct nclyr_win *win)
{
    struct manager_win *man = container_of(win, struct manager_win, super_win);

    man->printwin = cons_printf_compile("${if;selected:true}${reverse}${endif} ${win_num}: ${name}${right_align} ", ARRAY_SIZE(args), args);
}

static void manager_win_clean(struct nclyr_win *win)
{
    struct manager_win *man = container_of(win, struct manager_win, super_win);
    cons_printf_compiled_free(man->printwin);
}

static void manager_win_switch_to(struct nclyr_win *win)
{
    struct manager_win *man = container_of(win, struct manager_win, super_win);
    man->selected = 0;
    man->disp_offset = 0;
    win->updated = 1;
}

static void manager_win_update(struct nclyr_win *win)
{
    struct manager_win *man = container_of(win, struct manager_win, super_win);
    struct tui_iface *tui = win->tui;
    WINDOW *curwin = win->win;
    int i, cols, rows;

    win->updated = 0;
    werase(curwin);
    getmaxyx(curwin, rows, cols);

    for (i = 0; i < rows; i++) {
        if (i + man->disp_offset < tui->window_count) {
            struct nclyr_win *win = tui->windows[i + man->disp_offset];
            struct cons_str chstr;

            wmove(curwin, i, 0);

            args[0].u.str_val = win->win_name;
            args[1].u.int_val = i + man->disp_offset + 1;
            args[2].u.bool_val = ((i + man->disp_offset) == man->selected);

            cons_str_init(&chstr);
            cons_printf(man->printwin, &chstr, cols, tui_get_chtype_from_window(curwin), args, ARRAY_SIZE(args));
            waddchstr(curwin, chstr.chstr);
            cons_str_clear(&chstr);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

static void handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *event)
{
    struct manager_win *man = container_of(win, struct manager_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows;

    switch (ch) {
    case KEY_UP:
    case 'k':
        if (man->selected > 0) {
            man->selected--;
            if (man->selected < man->disp_offset)
                man->disp_offset--;
            win->updated = 1;
        }
        break;

    case KEY_DOWN:
    case 'j':
        if (man->selected < tui->window_count - 1) {
            man->selected++;
            rows = getmaxy(win->win);
            if (man->selected > man->disp_offset + rows - 1)
                man->disp_offset++;
            win->updated = 1;
        }
        break;

    case '\n':
        tui_change_window(tui, tui->windows[man->selected + man->disp_offset]);
        break;
    }
}

static void handle_window_select(struct nclyr_win *win, int ch, struct nclyr_mouse_event *event)
{
    switch (ch) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (ch - '1' < win->tui->window_count)
            tui_change_window(win->tui, win->tui->windows[ch - '1']);
        break;
    }
}

struct manager_win manager_win = {
    .super_win = {
        .win_name = "Window Manager",
        .win = NULL,
        .timeout = 400,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            N_KEYPRESS('j', handle_ch, "Select Window below"),
            N_KEYPRESS(KEY_DOWN, handle_ch, "Select Window below"),
            N_KEYPRESS('k', handle_ch, "Select Window above"),
            N_KEYPRESS(KEY_UP, handle_ch, "Select Window above"),
            N_KEYPRESS('1', handle_window_select, "Select Window 1"),
            N_KEYPRESS('2', handle_window_select, "Select Window 2"),
            N_KEYPRESS('3', handle_window_select, "Select Window 3"),
            N_KEYPRESS('4', handle_window_select, "Select Window 4"),
            N_KEYPRESS('5', handle_window_select, "Select Window 5"),
            N_KEYPRESS('6', handle_window_select, "Select Window 6"),
            N_KEYPRESS('7', handle_window_select, "Select Window 7"),
            N_KEYPRESS('8', handle_window_select, "Select Window 8"),
            N_KEYPRESS('9', handle_window_select, "Select Window 9"),
            N_KEYPRESS('\n', handle_ch, "Switch to selected window"),
            N_END()
        },
        .init = manager_win_init,
        .clean = manager_win_clean,
        .switch_to = manager_win_switch_to,
        .update = manager_win_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = NULL,
    },
    .selected = 0,
    .disp_offset = 0,
};

