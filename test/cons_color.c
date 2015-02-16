
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "cons_color.h"
#include "tui/tui_color.h"
#include "test/test.h"

static const char *color_names[] = { "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white", "default" };
static const char *color_names_caps[] = { "BLACK", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE", "DEFAULT" };

static const enum cons_color color_types[] = {
    CONS_COLOR_BLACK, CONS_COLOR_RED, CONS_COLOR_GREEN, CONS_COLOR_YELLOW,
    CONS_COLOR_BLUE, CONS_COLOR_MAGENTA, CONS_COLOR_CYAN, CONS_COLOR_WHITE,
    CONS_COLOR_DEFAULT,
};

int test_cols (void)
{
    int ret = 0;
    int i;
    enum cons_color col;

    for (i = 0; i < ARRAY_SIZE(color_names); i++) {
        col = cons_color_get(color_names[i]);
        ret += test_assert_with_name(color_names[i], (col == color_types[i]));
    }

    for (i = 0; i < ARRAY_SIZE(color_names); i++) {
        col = cons_color_get(color_names_caps[i]);
        ret += test_assert_with_name(color_names_caps[i], (col == color_types[i]));
    }

    /* Confirm that whatever name for the color cons_color_name gives us, it
     * can be fed back to cons_color_get() to get the correct color back */
    for (i = 0; i < ARRAY_SIZE(color_names); i++)
        ret += test_assert(color_types[i] ==  cons_color_get(cons_color_name(color_types[i])));

    return ret;
}

int tui_cols (void)
{
    int ret = 0;
    char buf[10];
    int i, k, n;
    struct cons_color_pair pair, pair2;

    for (i = 0; i < 8; i++) {
        for (k = 0; k < 8; k++) {
            pair.f = i;
            pair.b = k;

            n = tui_color_pair_get(&pair);
            tui_color_pair_fb(n, &pair2);

            sprintf(buf, "%d - %d,%d", n, i, k);

            if (test_assert_with_name(buf, pair2.f == pair.f)) {
                ret++;
                printf("  pair2.f: %d, pair.f: %d\n", pair2.f, pair.f);
            }

            if (test_assert_with_name(buf, pair2.b == pair.b)) {
                ret++;
                printf("  pair2.b: %d, pair.b: %d\n", pair2.b, pair.b);
            }
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
        { test_cols, "Color test" },
        { tui_cols,  "Tui color test" },
    };

    ret = run_tests("Test Console Colors", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}


