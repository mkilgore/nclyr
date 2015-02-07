#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "termcols.h"
#include "test/test.h"

static int global_test_number;
static int cur_test_count;
static int total_test_count = 0;
static const char *current_test;
static const char *current_mod;

int run_tests(const char *mod_name, struct unit_test *tests, int test_count, int argc, char **argv)
{
    int i;
    int error_count = 0;
    int errs;
    current_mod = mod_name;

    if (argc > 1)
        global_test_number = strtol(argv[1], NULL, 0);

    printf("==== Starting tests for %s ====\n", mod_name);

    for (i = 0; i < test_count; i++) {
        total_test_count++;

        cur_test_count = 0;
        current_test = tests[i].name;

        printf("== #%d: %s ==\n", total_test_count, tests[i].name);

        errs = (tests + i)->test ();

        printf("== Result: ");
        if (errs != 0)
            printf(COLOR_RED "FAIL -> %d" COLOR_RESET, errs);
        else
            printf(COLOR_GREEN "PASS" COLOR_RESET);

        printf(" ==\n");

        error_count += errs;
    }

    printf("==== Finished tests for %s ====\n", mod_name);
    printf("==== Result: ");
    if (error_count == 0)
        printf(COLOR_GREEN "PASS " COLOR_RESET);
    else
        printf(COLOR_RED "FAIL -> %d " COLOR_RESET, error_count);

    printf("====\n");

    return error_count;
}

int assert_true(const char *arg, const char *func, int cond)
{
    cur_test_count++;
    printf(" [%02d:%02d:%03d] %s: %s: ", global_test_number, total_test_count, cur_test_count, func, arg);
    if (cond)
        printf(COLOR_GREEN "PASS" COLOR_RESET);
    else
        printf(COLOR_RED "FAIL" COLOR_RESET);

    printf("\n");

    return !cond;
}

int assert_with_name(const char *name, const char *arg, const char *func, int cond)
{
    char buf[256];
    sprintf(buf, "%s: \"%s\"", name, arg);
    return assert_true(buf, func, cond);
}

