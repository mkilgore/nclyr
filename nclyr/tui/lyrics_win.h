#ifndef NCLYR_CURSES_LYRICS_WIN_H
#define NCLYR_CURSES_LYRICS_WIN_H

#include "window.h"

struct lyrics_win {
    struct nclyr_win super_win;

    size_t lines;
    size_t disp_offset;
    char **cur_lyrics;
};

extern struct lyrics_win lyrics_window;

#endif
