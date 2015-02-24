
#include "common.h"

#include <ncurses.h>

#include "cons/color.h"
#include "tui_color.h"
#include "debug.h"

static int color_map_curses[] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    -1,
};

void tui_color_init(void)
{
    const int colors[] = {
        CONS_COLOR_BLACK,
        CONS_COLOR_RED,
        CONS_COLOR_GREEN,
        CONS_COLOR_YELLOW,
        CONS_COLOR_BLUE,
        CONS_COLOR_MAGENTA,
        CONS_COLOR_CYAN,
        CONS_COLOR_WHITE,
        CONS_COLOR_DEFAULT,
    };
    int i, k;
    /* If it's 64 or less, then we assume it has standard default colors */
    int count = (COLOR_PAIRS <= 64)? 8: 9;

    DEBUG_PRINTF("Color pairs: %d\n", COLOR_PAIRS);
    for (i = 0; i < count; i++)
        for (k = 0; k < count; k++) {
            DEBUG_PRINTF("Pair: (%d, %d) = 0x%-08x\n", colors[i], colors[k], cons_color_pair_to_num(&(struct cons_color_pair) { colors[i], colors[k] }));
            init_pair(cons_color_pair_to_num( &(struct cons_color_pair) { colors[i], colors[k] }), color_map_curses[i], color_map_curses[k]);
        }
}

void tui_color_set(WINDOW *win, struct cons_color_pair colors)
{
    int rf = colors.f, rb = colors.b;
/*
    if (COLOR_PAIRS <= 64) {
        if (rf == CONS_COLOR_DEFAULT)
            rf = CONS_COLOR_WHITE;
        if (rb == CONS_COLOR_DEFAULT)
            rb = CONS_COLOR_BLACK;
    } */

    wattron(win, COLOR_PAIR(cons_color_pair_to_num(&(struct cons_color_pair) { rf, rb })));
}

void tui_color_unset(WINDOW *win, struct cons_color_pair colors)
{
    int rf = colors.f, rb = colors.b;

    /*
    if (COLOR_PAIRS <= 64) {
        if (rf == CONS_COLOR_DEFAULT)
            rf = CONS_COLOR_WHITE;
        if (rb == CONS_COLOR_DEFAULT)
            rb = CONS_COLOR_BLACK;
    }
    */

    wattroff(win, COLOR_PAIR(cons_color_pair_to_num(&(struct cons_color_pair) { rf, rb })));
}

void tui_color_pair_fb(int pair, struct cons_color_pair *c)
{
    if (pair != 0) {
        cons_color_num_to_pair(pair, c);
    } else {
        c->f = CONS_COLOR_DEFAULT;
        c->b = CONS_COLOR_DEFAULT;
    }
}

int tui_color_pair_get(struct cons_color_pair *c)
{
    if (c->f != CONS_COLOR_DEFAULT || c->b != CONS_COLOR_DEFAULT)
        return cons_color_pair_to_num(c);
    else
        return 0;
}

