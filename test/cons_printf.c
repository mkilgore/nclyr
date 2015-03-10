
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "cons/color.h"
#include "test/test.h"

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
    };

    ret = run_tests("Console printf", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}

