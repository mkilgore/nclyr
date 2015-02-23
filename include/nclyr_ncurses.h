#ifndef INCLUDE_NCLYR_NCURSES_H
#define INCLUDE_NCLYR_NCURSES_H

/* This file exists as a way of allowing us to use ncurses structures without
 * actually including ncurses.
 * This is nessisary because the cons folder uses some ncurses structures to
 * make it easy to use with the TUI, but we don't want to actually depend on
 * ncurses unless we're compiling the TUI in.
 *
 * Thus, we just make some fake ncurses settings that are compatable enough for
 * our uses. */

#if CONFIG_TUI
# include <ncurses.h>
#else

#include <stdint.h>

enum {
    FALSE,
    TRUE
};

typedef uint32_t chtype;

enum {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};

enum {
    A_NORMAL     = 0x00000000,
    A_STANDOUT   = 0x00010000,
    A_UNDERLINE  = 0x00020000,
    A_REVERSE    = 0x00040000,
    A_BLINK      = 0x00080000,
    A_DIM        = 0x00100000,
    A_BOLD       = 0x00200000,
    A_PROTECT    = 0x01000000,
    A_INVIS      = 0x00800000,

    A_CHARTEXT   = 0x000000FF,
    A_COLOR      = 0x0000FF00,
    A_ATTRIBUTES = 0xFFFFFF00
};

#define COLOR_PAIR(n) ((n) << 8)
#define PAIR_NUMBER(n) (((n) & A_COLOR) >> 8)

#endif /* CONFIG_TUI */

/* This is a flag to get hte attributes excluding the color pair */
enum {
    NCLYR_A_ATTRIBUTES_ONLY = (A_ATTRIBUTES & ~A_COLOR)
};

#endif
