
#include "common.h"

#include "stringcasecmp.h"
#include "cons/color.h"
#include "debug.h"

static const char *color_names[] = { "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white" };

static struct cons_color_pair default_colors = { CONS_COLOR_DEFAULT, CONS_COLOR_DEFAULT };

enum cons_color cons_color_get(const char *name)
{
    int i;
    for (i = 0; i < sizeof(color_names)/sizeof(*color_names); i++)
        if (stringcasecmp(color_names[i], name) == 0)
            return i;
    return CONS_COLOR_DEFAULT;
}

const char *cons_color_name(enum cons_color color)
{
    if (color == CONS_COLOR_DEFAULT)
        return "default";
    else
        return color_names[color];
}

void cons_color_set_default(struct cons_color_pair *pair)
{
    default_colors = *pair;
}

const struct cons_color_pair *cons_color_get_default(void)
{
    return &default_colors;
}

/* We flag our color numbers like this:
 *
 *
 * bit 7:     Foreground is default color
 * bit 6:     Background is default color
 * bit 5 - 3: Background color
 * Bit 2 - 0: Foreground color
 *
 * The foreground and background colors are placed in the low bits to allow for
 * terminals with COLOR_PAIRS <= 64 to use all the standard color pairs without
 * issue. When we have that number of COLOR_PAIRS though, we can't use the
 * default colors, and we also can't use (WHITE, WHITE), because we probably
 * only get 64 colors, but color 0 is not changeable, so we only actually get
 * 63.
 */

/* These macros do some helpful bit-flagging */
#define cons_color_pair_get_f_def(f) ((f) & (0x80))
#define cons_color_pair_get_b_def(f) ((f) & (0x40))

#define cons_color_pair_set_f_def(f, b) ((f) = (((f) & ~(0xB8)) | ((b) << 7)))
#define cons_color_pair_set_b_def(f,b)  ((f) = (((f) & ~(0x47)) | ((b) << 6)))

#define cons_color_set_fg(c, fg) (c) = (((c) & 0x47) | ((fg) << 3))
#define cons_color_set_bg(c, bg) (c) = (((c) & 0xB8) | (bg))

#define cons_color_get_fg(c) (((c) & 0x38) >> 3)
#define cons_color_get_bg(c) ((c) & 0x07)

int cons_color_pair_to_num(struct cons_color_pair *pair)
{
    int n = 0;
    if (pair->f == CONS_COLOR_DEFAULT && default_colors.f == CONS_COLOR_DEFAULT)
        cons_color_pair_set_f_def(n, 1);
    else if (pair->f == CONS_COLOR_DEFAULT)
        cons_color_set_fg(n, default_colors.f);
    else
        cons_color_set_fg(n, pair->f);

    if (pair->b == CONS_COLOR_DEFAULT && default_colors.b == CONS_COLOR_DEFAULT)
        cons_color_pair_set_b_def(n, 1);
    else if (pair->b == CONS_COLOR_DEFAULT)
        cons_color_set_bg(n, default_colors.b);
    else
        cons_color_set_bg(n, pair->b);

    return n + 1;
}

void cons_color_num_to_pair(int pair1, struct cons_color_pair *pair2)
{
    int n = pair1 - 1;

    if (cons_color_pair_get_f_def(n) && default_colors.f == CONS_COLOR_DEFAULT)
        pair2->f = CONS_COLOR_DEFAULT;
    else if (cons_color_pair_get_f_def(n))
        pair2->f = default_colors.f;
    else
        pair2->f = cons_color_get_fg(n);

    if (cons_color_pair_get_b_def(n) && default_colors.b == CONS_COLOR_DEFAULT)
        pair2->b = CONS_COLOR_DEFAULT;
    else if (cons_color_pair_get_b_def(n))
        pair2->b = default_colors.b;
    else
        pair2->b = cons_color_get_bg(n);
}

int cons_color_num_change_forground(int pair, int forc)
{
    pair--;
    if (pair == -1)
        pair = cons_color_pair_to_num(&default_colors) - 1;

    if (forc == CONS_COLOR_DEFAULT && default_colors.f == CONS_COLOR_DEFAULT)
        cons_color_pair_set_f_def(pair, 1);
    else if (forc == CONS_COLOR_DEFAULT)
        cons_color_set_fg(pair, default_colors.f);
    else
        cons_color_set_fg(pair, forc);

    pair++;
    return pair;
}

int cons_color_num_change_background(int pair, int backc)
{
    pair--;
    if (pair == -1)
        pair = cons_color_pair_to_num(&default_colors) - 1;

    if (backc == CONS_COLOR_DEFAULT && default_colors.b == CONS_COLOR_DEFAULT)
        cons_color_pair_set_b_def(pair, 1);
    else if (backc == CONS_COLOR_DEFAULT)
        cons_color_set_bg(pair, default_colors.b);
    else
        cons_color_set_bg(pair, backc);

    pair++;
    return pair;
}

