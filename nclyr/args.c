
#include "common.h"

#include <string.h>

#include "player.h"
#include "arg_parser.h"
#include "args.h"

static const char *arg_str = "[Flags] [Player] [Command]";
static const char *arg_desc_str  = "Player: The name of the player to attempt to connect too.\n"
                                   "Command: Optional command to send to player; If supplied, nclyr exits after\n"
                                   "         running that command\n";

static const char *version_text = "nclyr-" Q(NCLYR_VERSION_N) " Copyright (C) 2015 Matt Kilgore\n"
                                  "Build settings: " Q(NCLYR_BUILD_SETTINGS) "\n"
                                  "\n"
                                  "This is free software: you are free to change and redistribute it.\n"
                                  "There is NO WARRANTY, to the extent permitted by law.\n";

enum arg_index {
    ARG_EXTRA = ARG_PARSER_EXTRA,
    ARG_ERR = ARG_PARSER_ERR,
    ARG_DONE = ARG_PARSER_DONE,
#define X(enu, ...) ARG_ENUM(enu)
    NCLYR_DEFAULT_ARGS,
#undef X
    ARG_LAST
};

static const struct arg args[] = {
#define X(...) CREATE_ARG(__VA_ARGS__)
    NCLYR_DEFAULT_ARGS,
#undef X
    { NULL }
};

int parse_args(int argc, char **argv, const char **player_name, const char **command)
{
    struct player **player;
    enum arg_index ret;

    while ((ret = arg_parser(argc, argv, args)) != ARG_DONE) {
        switch (ret) {
        case ARG_help:
            display_help_text(argv[0], arg_str, arg_desc_str, args);
            return 1;
        case ARG_version:
            printf("%s", version_text);
            return 1;

        case ARG_list_players:
            printf("Available players:\n");
            for (player = players; *player; player++)
                printf("  - %s\n", (*player)->name);
            return 1;

        case ARG_EXTRA:
            if (*player_name == NULL)
                *player_name = argarg;
            else
                *command = argarg;
            break;

        case ARG_ERR:
            return 1;

        case ARG_LAST:
        case ARG_DONE:
            /* Shouldn't ever get here */
            return 0;
        }
    }

    return 0;
}

