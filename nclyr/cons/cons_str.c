
#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "cons/color.h"
#include "cons/str.h"
#include "debug.h"

#define CHTYPE_STATE_ALLOC_STEP 1024

static inline int get_next_block_size(int new_bytes)
{
    return (new_bytes + CHTYPE_STATE_ALLOC_STEP - 1) & ~(CHTYPE_STATE_ALLOC_STEP - 1);
}

static inline void cons_str_resize(struct cons_str *chstr, int new_len)
{
    if ((new_len + 1) * sizeof(*chstr->chstr) >= chstr->alloced) {
        chstr->alloced = get_next_block_size((new_len + 1) * sizeof(*chstr->chstr));
        chstr->chstr = realloc(chstr->chstr, chstr->alloced);
    }
}

void cons_str_init(struct cons_str *chstr)
{
    memset(chstr, 0, sizeof(*chstr));
}

void cons_str_clear(struct cons_str *chstr)
{
    free(chstr->chstr);
}

void cons_str_setwidth(struct cons_str *chstr, int width)
{
    chstr->max_width = width;
    chstr->alloced = sizeof(*chstr->chstr) * (width + 1);
    chstr->chstr = realloc(chstr->chstr, chstr->alloced);
    memset(chstr->chstr, 0, chstr->alloced);
}

void cons_str_add_cons_str(struct cons_str *dest, const struct cons_str *src)
{
    chtype *cur;
    chtype *str = src->chstr;
    int len = src->length;

    if (!src->chstr)
        return ;

    if (dest->max_width == 0) {
        cons_str_resize(dest, dest->length + len);

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

void cons_str_add_str(struct cons_str *chstr, const char *str, cons_attr attrs)
{
    chtype *cur;
    int len = strlen(str);

    if (chstr->max_width == 0) {
        cons_str_resize(chstr, chstr->length + len);

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

void cons_str_add_cons_str_at(struct cons_str *dest, const struct cons_str *src, int at)
{
    if (dest->length > at) {
        dest->length = at;
        dest->chstr[dest->length] = '\0';
    } else if (dest->length < at) {
        cons_attr attr = dest->chstr[dest->length - 1] & A_ATTRIBUTES;
        cons_str_resize(dest, at);

        for (; dest->length < at; dest->length++)
            dest->chstr[dest->length] = ' ' | attr;

        dest->chstr[dest->length] = '\0';
    }

    cons_str_add_cons_str(dest, src);
}

void cons_str_add_ch(struct cons_str *chstr, cons_char ch)
{
    cons_str_resize(chstr, chstr->length + 1);
    chstr->length++;

    chstr->chstr[chstr->length - 1] = ch;
    chstr->chstr[chstr->length] = '\0';
}

