
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "cons/color.h"
#include "cons/str.h"
#include "test/test.h"

int compare_cons_str(const struct cons_str *str1, const struct cons_str *str2)
{
    cons_char *c1 = str1->chstr, *c2 = str2->chstr;
    while (*c1 && *c2)
        if (*c1 != *c2)
            return 1;

    if (*c1 || *c2)
        return 1;

    return 0;
}

#define assert_cons_str_equal(str1, str2) \
    test_assert(compare_cons_str(str1, str2) == 0)

#define TEST_ARR { 'T', 'E', 'S', 'T', '\0' }

int cons_str_test(void)
{
    int ret = 0;
    struct cons_str cmp;
    struct cons_str chstr;
    cons_char *c;

    cons_str_init(&chstr);
    cons_str_init(&cmp);

    cmp.chstr = (cons_char []) TEST_ARR;
    cons_str_add_str(&chstr, (char []) TEST_ARR, 0);

    printf("Cmp: ");
    for (c = cmp.chstr; *c; c++)
        putchar(*c);
    putchar('\n');
    printf("chstr: ");
    for (c = chstr.chstr; *c; c++)
        putchar(*c);
    putchar('\n');

    ret += assert_cons_str_equal(&chstr, &cmp);

    cons_str_clear(&chstr);

    return ret;
}

int cons_str_width_test(void)
{
    int ret = 0;

    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
        { cons_str_test, "cons_str_add" },
        { cons_str_width_test, "cons_str_setwidth" },
    };

    ret = run_tests("Console String", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}

