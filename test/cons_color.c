
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "cons_color.h"
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

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
        { test_cols, "Color test" },
    };

    ret = run_tests("Test Console Colors", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}


