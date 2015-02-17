#ifndef NCLYR_CURSES_PLAYLIST_WIN_H
#define NCLYR_CURSES_PLAYLIST_WIN_H

#include "tui_printf.h"
#include "windows/window.h"
#include "playlist.h"

struct playlist_win {
    struct nclyr_win super_win;

    int selected;
    int disp_offset;

    tui_printf_compiled *printline;
};

extern struct playlist_win playlist_window;

#endif
