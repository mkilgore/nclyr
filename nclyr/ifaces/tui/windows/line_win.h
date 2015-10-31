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
void line_handle_mouse(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent);

void line_free_lines(struct line_win *);

#define LINE_KEYPRESSES() \
    NCLYR_KEYPRESS('j', line_handle_keypress, "Scroll down"), \
    NCLYR_KEYPRESS('k', line_handle_keypress, "Scroll up"), \
    NCLYR_KEYPRESS(KEY_DOWN, line_handle_keypress, "Scroll down"), \
    NCLYR_KEYPRESS(KEY_UP, line_handle_keypress, "Scroll up"), \
    NCLYR_KEYPRESS('J', line_handle_keypress, "Scroll down one page"), \
    NCLYR_KEYPRESS('K', line_handle_keypress, "Scroll up one page"), \
    NCLYR_KEYPRESS(KEY_LEFT, line_handle_keypress, "Scroll down one page"), \
    NCLYR_KEYPRESS(KEY_RIGHT, line_handle_keypress, "Scroll up one page"), \
    NCLYR_KEYPRESS(KEY_NPAGE, line_handle_keypress, "Scroll down one page"), \
    NCLYR_KEYPRESS(KEY_PPAGE, line_handle_keypress, "Scroll up one page"), \
    NCLYR_MOUSE(SCROLL_UP, line_handle_mouse, "Scroll up"), \
    NCLYR_MOUSE(SCROLL_DOWN, line_handle_mouse, "Scroll down")

#endif
