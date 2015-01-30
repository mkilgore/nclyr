
#include "common.h"

#include "a_sprintf.h"
#include "build.h"
#include "debug.h"

struct build_option build_options[] = {
    { .name = "glyr", .build = CONFIG_LIB_GLYR },
    { .name = "tui", .build = CONFIG_TUI },
    { .name = "console", .build = CONFIG_CONSOLE },
    { .name = "pianobar", .build = CONFIG_PLAYER_PIANOBAR },
    { .name = "mpd", .build = CONFIG_PLAYER_MPD },
    { .name = NULL }
};

void build_settings_print(FILE *file)
{
    struct build_option *opt;
    printf("Build settings: ");
    for (opt = build_options; opt->name != NULL; opt++)
        printf("%c%s ", (opt->build)? '+': '-', opt->name);
}

