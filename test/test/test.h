#ifndef TEST_TEST_H
#define TEST_TEST_H

struct unit_test {
    int (*test)(void);
    const char *name;
};

#define TEST_COLOR_DEFAULT      ""
#define TEST_COLOR_RESET        "\033[m"
#define TEST_COLOR_BOLD         "\033[1m"
#define TEST_COLOR_RED          "\033[31m"
#define TEST_COLOR_GREEN        "\033[32m"
#define TEST_COLOR_YELLOW       "\033[33m"
#define TEST_COLOR_BLUE         "\033[34m"
#define TEST_COLOR_MAGENTA      "\033[35m"
#define TEST_COLOR_CYAN         "\033[36m"
#define TEST_COLOR_BOLD_RED     "\033[1;31m"
#define TEST_COLOR_BOLD_GREEN   "\033[1;32m"
#define TEST_COLOR_BOLD_YELLOW  "\033[1;33m"
#define TEST_COLOR_BOLD_BLUE    "\033[1;34m"
#define TEST_COLOR_BOLD_MAGENTA "\033[1;35m"
#define TEST_COLOR_BOLD_CYAN    "\033[1;36m"
#define TEST_COLOR_BG_RED       "\033[41m"
#define TEST_COLOR_BG_GREEN     "\033[42m"
#define TEST_COLOR_BG_YELLOW    "\033[43m"
#define TEST_COLOR_BG_BLUE      "\033[44m"
#define TEST_COLOR_BG_MAGENTA   "\033[45m"
#define TEST_COLOR_BG_CYAN      "\033[46m"

#define test_assert(cond) assert_true(Q(#cond), __func__, !!(cond))
#define test_assert_with_name(name, cond) assert_with_name(name, #cond, __func__, !!(cond))

extern int run_tests(const char *test_mod_name, struct unit_test *, int test_count, int argc, char **argv);
extern int assert_true(const char *arg, const char *func, int cond);
extern int assert_with_name(const char *name, const char *arg, const char *func, int cond);

#endif
