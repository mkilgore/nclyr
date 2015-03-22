#ifndef NCLYR_CURSES_PLAYLIST_WIN_H
#define NCLYR_CURSES_PLAYLIST_WIN_H

#include "cons/printf.h"
#include "windows/window.h"
#include "playlist.h"

struct playlist_win {
    struct nclyr_win super_win;

    int selected;
    int disp_offset;

    cons_printf_compiled *printline;
    cons_printf_compiled *song_triple;
    cons_printf_compiled *song_filename;
};

struct nclyr_win *playlist_win_new(void);

#endif
