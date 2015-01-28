
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <signal.h>
#include <pthread.h>
#include <glyr/glyr.h>

#include "player.h"
#include "song.h"
#include "lyr_thread.h"
#include "signal_handler.h"
#include "tui.h"
#include "config.h"
#include "nclyr_conf.h"
#include "args.h"
#include "debug.h"

static int arg_handle(struct arg_parser *, int index, const char *arg);

struct arg_parser_extra {
    struct arg_parser parser;
    const char *player;
    const char *command;
};

static struct arg_parser_extra args = {
    .parser = {
        .args = nclyr_args,
        .arg_callback = arg_handle,
        .arg_str = "[Flags] [Player] [Command]",
        .arg_desc_str = "Player: The name of the player to attempt to connect too.\n"
                        "Command: Optional command to send to player; If supplied, nclyr exits after\n"
                        "         running that command\n",
    },
    .player = NULL,
    .command = NULL
};

static const char *version_text = "nclyr-" Q(NCLYR_VERSION_N) " Copyright (C) 2015 Matt Kilgore\n"
                                  "Build settings: " Q(NCLYR_BUILD_SETTINGS) "\n"
                                  "\n"
                                  "This is free software: you are free to change and redistribute it.\n"
                                  "There is NO WARRANTY, to the extent permitted by law.\n";


static int arg_handle(struct arg_parser *parser, int index, const char *arg)
{
    struct arg_parser_extra *extra = container_of(parser, struct arg_parser_extra, parser);
    struct player **player;

    printf("In arg handle!\n");

    switch (index) {
    case ARG_help:
        config_disp_small_helptext(&nclyr_config, parser);
        return 1;
    case ARG_help_all:
        config_disp_full_helptext(&nclyr_config, parser);
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
        DEBUG_PRINTF("Extra argument: %s\n", arg);
        if (extra->player == NULL)
            extra->player = arg;
        else
            extra->command = arg;
        break;

    case ARG_config:
        /* We parse to handle ARG_config earlier so we can load the config
         * file before actually parsing the arguments.  This is so the
         * arguments supplied can override whatever is in the configuration
         * file. Because of this, there is nothing to do here. */
        break;

    case ARG_ERR:
        return 1;

    case ARG_LAST:
    case ARG_DONE:
        /* Shouldn't ever get here */
        return 1;
    }

    return 0;
}

int main(int argc, const char **argv)
{
    const char *config = NULL;
    int pipefd[2], notifyfd[2], signalfd[2];
    struct song_info *song = NULL;
    sigset_t set;

    DEBUG_INIT();

    DEBUG_PRINTF("nclyr started!\n");

    args.parser.argc = argc;
    args.parser.argv = argv;

    nclyr_conf_init();

    config_check_for_config(argc, argv, &config);

    if (!config)
        config = "./test_config";

    DEBUG_PRINTF("Config file: %s\n", argv[0], config);

    if (config_load_from_file(&nclyr_config, config) != 0) {
        printf("%s: Error reading config file '%s'\n", argv[0], config);
        return 1;
    }

    if (config_load_from_args(&nclyr_config, &args.parser) != 0) {
        printf("%s: Something happened when parsing args\n", argv[0]);
        return 1;
    }

    config_print(&nclyr_config);

    nclyr_conf_clear();
    DEBUG_CLOSE();
    return 0;

    if (!args.player)
        args.player = "mpd";

    player_set_current(player_find(args.player));

    if (player_current() == NULL) {
        printf("%s: Error, no music player selected.\n", argv[0]);
        return 0;
    }

    /* Open up three pipes that we'll use to communicate with the three main
     * threads */
    pipe(pipefd);
    pipe(notifyfd);
    pipe(signalfd);

    /* Start a thread to catch all of our signals and send them down the
     * signalfd pipe. This thread has to be started before we mask all the
     * signals on the main thread */
    signal_start_handler(signalfd[1]);

    /* Make sure no threads but the signal thread recieves any signals by
     * masking them on the main thread. */
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    lyr_thread_start(notifyfd[1]);
    player_start_thread(player_current(), pipefd[1]);

    tui_main_loop(signalfd[0], pipefd[0], notifyfd[0]);

    player_stop_thread(player_current());
    lyr_thread_stop();
    signal_stop_handler();

    song_clear(song);
    free(song);

    nclyr_conf_clear();

    DEBUG_CLOSE();

    return 0;
}

