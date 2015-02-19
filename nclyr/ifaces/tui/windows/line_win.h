#ifndef NCLYR_CURSES_LINE_WIN_H
#define NCLYR_CURSES_LINE_WIN_H

#include "player.h"
#include "window.h"

struct line_win {
    struct nclyr_win super_win;

    size_t line_count;
    size_t disp_offset;
    char **lines;

    unsigned int center:1;
};

void line_update(struct nclyr_win *);
void line_clean(struct nclyr_win *);
void line_handle_keypress(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent);

void line_free_lines(struct line_win *);

#define LINE_KEYPRESSES() \
    N_KEYPRESS('j', line_handle_keypress, "Scroll down"), \
    N_KEYPRESS('k', line_handle_keypress, "Scroll up"), \
    N_KEYPRESS('J', line_handle_keypress, "Scroll down one page"), \
    N_KEYPRESS('K', line_handle_keypress, "Scroll up one page"), \
    N_KEYPRESS(KEY_NPAGE, line_handle_keypress, "Scroll down one page"), \
    N_KEYPRESS(KEY_PPAGE, line_handle_keypress, "Scroll up one page")

#endif
