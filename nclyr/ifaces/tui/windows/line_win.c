
#include "common.h"

#include <string.h>

#include "song.h"
#include "player.h"
#include "tui_internal.h"
#include "window.h"
#include "line_win.h"

void line_update (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    WINDOW *curwin = win->win;
    int i, rows, cols;
    werase(curwin);

    getmaxyx(curwin, rows, cols);
    for (i = 0; i < rows && i + line->disp_offset < line->line_count; i++) {
        const char *l = line->lines[i + line->disp_offset];
        if (!line->center)
            mvwprintw(curwin, i, 0, "%-*s", cols, l);
        else
            mvwprintw(curwin, i, cols / 2 - strlen(l) / 2, "%s", l);
    }
}

void line_free_lines (struct line_win *line)
{
    int i;
    for (i = 0; i < line->line_count; i++)
        free(line->lines[i]);

    free(line->lines);

    line->line_count = 0;
    line->disp_offset = 0;
    line->lines = NULL;
}

void line_clean (struct nclyr_win *win)
{
    struct line_win *line = container_of(win, struct line_win, super_win);

    line_free_lines(line);
}

void line_handle_mouse(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    /* We just 'map' the mouse events to the coresponding keypress event, to
     * avoid duplicating the logic. */
    switch (mevent->type) {
    case SCROLL_UP:
        line_handle_keypress(win, 'k', NULL);
        break;
    case SCROLL_DOWN:
        line_handle_keypress(win, 'j', NULL);
        break;
    default:
        break;
    }
}

void line_handle_keypress(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct line_win *line = container_of(win, struct line_win, super_win);
    WINDOW *curwin = win->win;
    int rows;

    rows = getmaxy(curwin);

    switch (ch) {
    case 'j':
        if (line->disp_offset < line->line_count - 1) {
            line->disp_offset++;
            win->updated = 1;
        }
        break;
    case 'k':
        if (line->disp_offset > 0) {
            line->disp_offset--;
            win->updated = 1;
        }
        break;
    case 'J':
    case KEY_NPAGE:
        if (line->disp_offset < line->line_count - 1) {
            if (rows < ((line->line_count - 1) - line->disp_offset))
                line->disp_offset += rows;
            else
                line->disp_offset = line->line_count - 1;
            win->updated = 1;
        }
        break;
    case 'K':
    case KEY_PPAGE:
        if (line->disp_offset > 0) {
            if (rows < line->disp_offset)
                line->disp_offset -= rows;
            else
                line->disp_offset = 0;
            win->updated = 1;
        }
        break;
    }
}

