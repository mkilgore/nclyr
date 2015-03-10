
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "termcols.h"
#include "cons/color.h"
#include "cons/str.h"
#include "console_cons_str.h"

struct str {
    int alloced;
    int length;
    char *str;
};

#define STR_ALLOC_STEP 256

static inline int get_next_block_size(int new_bytes)
{
    return (new_bytes + STR_ALLOC_STEP - 1) & ~(STR_ALLOC_STEP - 1);
}

static inline void str_resize(struct str *chstr, int new_len)
{
    if ((new_len + 1) * sizeof(*chstr->str) >= chstr->alloced) {
        chstr->alloced = get_next_block_size((new_len + 1) * sizeof(*chstr->str));
        chstr->str = realloc(chstr->str, chstr->alloced);
    }
}

static void str_add_str(struct str *str, const char *str2)
{
    char *cur;
    int len = strlen(str2);

    str_resize(str, str->length + len);

    for (cur = str->str + str->length; *str2; str2++, cur++)
        *cur = *str2;
    *cur = '\0';

    str->length += len;
}

static void str_add_ch(struct str *str, char ch)
{
    str_resize(str, str->length + 1);
    str->length++;

    str->str[str->length - 1] = ch;
    str->str[str->length] = '\0';
}

static const char *fg_colors[] = {
#define X(c) [CONS_COLOR_ ## c] = TERM_COLOR_ ## c
    X(BLACK),
    X(RED),
    X(GREEN),
    X(YELLOW),
    X(BLUE),
    X(MAGENTA),
    X(CYAN),
    X(WHITE),
#undef X
};

static const char *bg_colors[] = {
#define X(c) [CONS_COLOR_ ## c] = TERM_COLOR_BG_ ## c
    X(BLACK),
    X(RED),
    X(GREEN),
    X(YELLOW),
    X(BLUE),
    X(MAGENTA),
    X(CYAN),
    X(WHITE),
#undef X
};

static void console_apply_attrs(cons_attr *cur_attr, cons_attr attrs)
{

}

char *console_cons_str_print(struct cons_str *chstr)
{
    struct str ret = { 0 };
    chtype *ch;
    struct cons_color_pair pair;
    struct cons_color_pair cur_color = { CONS_COLOR_DEFAULT, CONS_COLOR_DEFAULT };
    cons_attr cur_attr = 0;

    for (ch = chstr->chstr; *ch; ch++) {
        int col = PAIR_NUMBER(*ch);
        cons_color_num_to_pair(col, &pair);

        if (pair.f != cur_color.f) {
            cur_color.f = pair.f;
            if (cur_color.f != CONS_COLOR_DEFAULT)
                str_add_str(&ret, fg_colors[cur_color.f]);
            else
                str_add_str(&ret, TERM_COLOR_DEFAULT);
        }

        if (pair.b != cur_color.b) {
            cur_color.b = pair.b;
            if (cur_color.b != CONS_COLOR_DEFAULT)
                str_add_str(&ret, bg_colors[cur_color.b]);
            else
                str_add_str(&ret, TERM_COLOR_BG_DEFAULT);
        }

        console_apply_attrs(&cur_attr, *ch & NCLYR_A_ATTRIBUTES_ONLY);

        str_add_ch(&ret, *ch & A_CHARTEXT);
    }

    if (cur_color.f != CONS_COLOR_DEFAULT)
        str_add_str(&ret, TERM_COLOR_DEFAULT);

    if (cur_color.b != CONS_COLOR_DEFAULT)
        str_add_str(&ret, TERM_COLOR_BG_DEFAULT);

    return ret.str;
}
