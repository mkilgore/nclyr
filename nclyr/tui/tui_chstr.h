#ifndef NCLYR_TUI_CHSTR_H
#define NCLYR_TUI_CHSTR_H

#include <ncurses.h>

struct chstr {
    int max_width;
    int alloced, length;
    chtype *chstr;
};

void chstr_addchstr_at(struct chstr *, const struct chstr *str, int at);

void chstr_addchstr(struct chstr *dest, const struct chstr *src);
void chstr_addstr(struct chstr *, const char *str, chtype attrs);
void chstr_addch(struct chstr *, int ch, chtype attrs);
void chstr_setwidth(struct chstr *, int width);

void chstr_init(struct chstr *);
void chstr_clear(struct chstr *);

#endif
