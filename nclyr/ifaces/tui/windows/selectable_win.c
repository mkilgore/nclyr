
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
        /* tui->state.playlist.song_count */
        if (sel->selected < sel->total_lines - 1) {
            sel->selected++;
            rows = getmaxy(win->win);
            if (sel->selected > sel->disp_offset + rows - 1)
                sel->disp_offset++;
            win->updated = 1;
        }
        break;

#if 0
    case 'd':
        player_remove_song(player_current(), sel->selected);
        break;
#endif

    case KEY_LEFT:
    case 'J':
    case KEY_NPAGE:
        if (sel->disp_offset < sel->total_lines - 1) {
            rows = getmaxy(win->win);
            if (rows < ((sel->total_lines - 1) - sel->disp_offset))
                sel->disp_offset += rows;
            else
                sel->disp_offset = sel->total_lines - 1;
            win->updated = 1;
        }
        break;
    case KEY_RIGHT:
    case 'K':
    case KEY_PPAGE:
        if (sel->disp_offset > 0) {
            rows = getmaxy(win->win);
            if (rows < sel->disp_offset)
                sel->disp_offset -= rows;
            else
                sel->disp_offset = 0;
            win->updated = 1;
        }
        break;
#if 0
    case '\n':
        (sel->line_selected) (sel, sel->selected);
        break;
#endif
    }

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
        sel->selected = sel->disp_offset + mevent->y;
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

            (sel->get_line) (sel, i + sel->disp_offset, (i + sel->disp_offset) == sel->selected, &line);
            mvwaddchstr(curwin, i, 0, line.chstr);

            cons_str_clear(&line);
#if 0
            struct tui_iface *tui = win->tui;
            struct cons_str chstr;
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
#endif
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

