
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
#include "window.h"
#include "selectable_win.h"
#include "debug.h"

void selectable_win_handle_ch(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    int rows;

    rows = getmaxy(win->win);

    switch (ch) {
    case KEY_UP:
    case 'k':
        if (sel->selected > 0) {
            sel->selected--;
            if (sel->selected < sel->disp_offset)
                sel->disp_offset--;
            win->updated = 1;
        }
        break;

    case KEY_DOWN:
    case 'j':
        if (sel->selected < sel->total_lines - 1) {
            sel->selected++;
            if (sel->selected > sel->disp_offset + rows - 1)
                sel->disp_offset++;
            win->updated = 1;
        }
        break;

    case KEY_NPAGE:
    case K_CONTROL('f'):
        if (sel->disp_offset < sel->total_lines - 1) {
            if (rows < ((sel->total_lines - 1) - sel->disp_offset))
                sel->disp_offset += rows;
            else
                sel->disp_offset = sel->total_lines - 1;
            win->updated = 1;
        }
        break;

    case KEY_PPAGE:
    case K_CONTROL('b'):
        if (sel->disp_offset > 0) {
            if (rows < sel->disp_offset)
                sel->disp_offset -= rows;
            else
                sel->disp_offset = 0;
            win->updated = 1;
        }
        break;

    case K_CONTROL('d'):
        if (sel->disp_offset < sel->total_lines - 1) {
            int half = rows / 2;

            if (half < ((sel->total_lines - 1) - sel->disp_offset))
                sel->disp_offset += half;
            else
                sel->disp_offset = sel->total_lines - 1;

            sel->selected += half;

            if (sel->selected >= sel->total_lines)
                sel->selected = sel->total_lines - 1;

            win->updated = 1;
        }
        break;

    case K_CONTROL('u'):
        if (sel->disp_offset > 0) {
            int half = rows / 2;

            if (half < sel->disp_offset)
                sel->disp_offset -= half;
            else
                sel->disp_offset = 0;

            sel->selected -= half;

            if (sel->selected < 0)
                sel->selected = 0;

            win->updated = 1;
        }
        break;

    case 'g':
        sel->selected = 0;
        sel->disp_offset = 0;
        win->updated = 1;
        break;

    case 'G':
        sel->selected = sel->total_lines - 1;
        sel->disp_offset = sel->selected - rows + 1;

        if (sel->disp_offset < 0)
            sel->disp_offset = 0;

        win->updated = 1;
        break;
    }

    if (sel->selected < sel->disp_offset)
        sel->selected = sel->disp_offset;
    else if (sel->selected - sel->disp_offset > rows)
        sel->selected = sel->disp_offset + rows - 1;
}

void selectable_win_handle_mouse(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);

    if (ch != KEY_MOUSE || !mevent)
        return ;

    DEBUG_PRINTF("Selectable Mouse event: %d, (%d, %d)\n", mevent->type, mevent->x, mevent->y);

    if (mevent->type == SCROLL_UP) {
        if (sel->disp_offset > 0) {
            sel->disp_offset--;
            win->updated = 1;
        }
    } else if (mevent->type == SCROLL_DOWN) {
        if (sel->disp_offset < sel->total_lines - 1) {
            sel->disp_offset++;
            win->updated = 1;
        }
    } else if (mevent->type == LEFT_CLICKED) {
        int new_sel;
        new_sel = sel->disp_offset + mevent->y;

        if (new_sel > sel->total_lines - 1)
            new_sel = sel->total_lines - 1;

        if (sel->selected == new_sel)
            (sel->line_selected) (sel);
        else
            sel->selected = new_sel;

        win->updated = 1;
    }
}

void selectable_win_update(struct nclyr_win *win)
{
    struct selectable_win *sel = container_of(win, struct selectable_win, super_win);
    int rows, cols, i;
    WINDOW *curwin = win->win;

    win->updated = 0;

    werase(curwin);

    getmaxyx(curwin, rows, cols);

    for (i = 0; i < rows; i++) {
        if (sel->total_lines > i + sel->disp_offset) {
            struct cons_str line;

            cons_str_init(&line);

            (sel->get_line) (sel, i + sel->disp_offset, cols, &line);
            mvwaddchstr(curwin, i, 0, line.chstr);

            cons_str_clear(&line);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

