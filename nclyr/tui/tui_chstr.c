

#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "tui_color.h"
#include "tui_chstr.h"
#include "debug.h"

#define CHTYPE_STATE_ALLOC_STEP 1024

static inline int get_next_block_size(int new_bytes)
{
    return (new_bytes + CHTYPE_STATE_ALLOC_STEP - 1) & ~(CHTYPE_STATE_ALLOC_STEP - 1);
}

static inline void chstr_resize(struct chstr *chstr, int new_len)
{
    if ((new_len + 1) * sizeof(*chstr->chstr) >= chstr->alloced) {
        chstr->alloced = get_next_block_size((new_len + 1) * sizeof(*chstr->chstr));
        chstr->chstr = realloc(chstr->chstr, chstr->alloced);
    }
}

void chstr_init(struct chstr *chstr)
{
    memset(chstr, 0, sizeof(*chstr));
}

void chstr_clear(struct chstr *chstr)
{
    free(chstr->chstr);
}

void chstr_setwidth(struct chstr *chstr, int width)
{
    chstr->max_width = width;
    chstr->alloced = sizeof(*chstr->chstr) * (width + 1);
    chstr->chstr = realloc(chstr->chstr, chstr->alloced);
    memset(chstr->chstr, 0, chstr->alloced);
}

void chstr_addchstr(struct chstr *dest, const struct chstr *src)
{
    chtype *cur;
    chtype *str = src->chstr;
    int len = src->length;

    if (!src->chstr)
        return ;

    if (dest->max_width == 0) {
        chstr_resize(dest, dest->length + len);

        for (cur = dest->chstr + dest->length; *str; str++, cur++)
            *cur = *str;
    } else {
        for (cur = dest->chstr + dest->length;
             *str && (cur - dest->chstr) < dest->max_width;
             str++, cur++)
            *cur = *str;
    }

    *cur = '\0';

    dest->length += len;
}

void chstr_addstr(struct chstr *chstr, const char *str, chtype attrs)
{
    chtype *cur;
    int len = strlen(str);

    if (chstr->max_width == 0) {
        chstr_resize(chstr, chstr->length + len);

        for (cur = chstr->chstr + chstr->length; *str; str++, cur++)
            *cur = *str | attrs;
        *cur = '\0';
    } else if (chstr->length < chstr->max_width) {
        for (cur = chstr->chstr + chstr->length;
             *str && ((cur - chstr->chstr) < chstr->max_width - 1);
             str++, cur++)
            *cur = *str | attrs;
        *cur = '\0';
    }


    chstr->length += len;
}

void chstr_addchstr_at(struct chstr *dest, const struct chstr *src, int at)
{
    if (dest->length > at) {
        dest->length = at;
        dest->chstr[dest->length] = '\0';
    } else if (dest->length < at) {
        chtype attr = dest->chstr[dest->length - 1] & A_ATTRIBUTES;
        chstr_resize(dest, at);

        for (; dest->length < at; dest->length++)
            dest->chstr[dest->length] = ' ' | attr;

        dest->chstr[dest->length] = '\0';
    }

    chstr_addchstr(dest, src);
}

void chstr_addch(struct chstr *chstr, int ch, chtype attrs)
{
    chstr_resize(chstr, chstr->length + 1);
    chstr->length++;

    chstr->chstr[chstr->length - 1] = ch | attrs;
    chstr->chstr[chstr->length] = '\0';
}

