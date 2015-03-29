
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
#include "config.h"
#include "nclyr_conf.h"
#include "thread_com.h"
#include "args.h"
#include "iface.h"
#include "build.h"
#include "debug.h"

static int arg_handle(struct arg_parser *, int index, const char *arg);

struct arg_parser_extra {
    struct arg_parser parser;
    const char *command;
};

static struct arg_parser_extra args = {
    .parser = {
        .args = nclyr_args,
        .arg_callback = arg_handle,
        .arg_str = "[Flags] [Command]",
        .arg_desc_str = "Command: Optional command to send to player; If supplied, nclyr exits after\n"
                        "         running that command\n",
    },
    .command = NULL,
};

static const char *version_text = "nclyr-" Q(NCLYR_VERSION_N) " Copyright (C) 2015 Matt Kilgore\n";

static const char *license_text = "This is free software: you are free to change and redistribute it.\n"
                                  "There is NO WARRANTY, to the extent permitted by law.\n";


static int arg_handle(struct arg_parser *parser, int index, const char *arg)
{
    struct arg_parser_extra *extra = container_of(parser, struct arg_parser_extra, parser);
    struct player **player;
    struct nclyr_iface **iface;
    const char *def_iface;

    switch (index) {
    case ARG_help:
        config_disp_small_helptext(&nclyr_config, parser);
        config_disp_root_help(&nclyr_config);
        return 1;
    case ARG_version:
        printf("%s", version_text);
        build_settings_print(stdout);
        putchar('\n');
        putchar('\n');
        printf("%s", license_text);
        return 1;

    case ARG_list_players:
        printf("Available players:\n");
        for (player = players; *player; player++)
            printf("  - %s\n", (*player)->name);
        return 1;

    case ARG_list_interfaces:
        def_iface = ROOT_CONFIG(NCLYR_CONFIG_INTERFACE)->u.str.str;
        printf("Available interfaces:\n");
        for (iface = nclyr_iface_list; *iface; iface++)
            printf("  - %s: %s%s\n", (*iface)->name, (*iface)->description, (strcmp((*iface)->name, def_iface) == 0)? " (default)":"");
        return 1;

    case ARG_help_config:
        config_disp_complete_configtext(&nclyr_config);
        return 1;

    case ARG_EXTRA:
        DEBUG_PRINTF("Extra argument: %s\n", arg);
        if (extra->command == NULL)
            extra->command = arg;
        break;

    case ARG_no_config:
        /* Handled in the earlier parsing for config */
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
    const char *config = CONFIG_CONFIG_DEFAULT;
    struct nclyr_pipes pipes;
    struct nclyr_iface *iface;
    sigset_t set;

    DEBUG_INIT();

    DEBUG_PRINTF("nclyr started!\n");

    args.parser.argc = argc;
    args.parser.argv = argv;

    config_check_for_config(argc, argv, &config);

    ROOT_CONFIG(NCLYR_CONFIG_CONFIG)->u.str.str = (char *)config;

    DEBUG_PRINTF("Config file: %s\n", argv[0], ROOT_CONFIG(NCLYR_CONFIG_CONFIG)->u.str.str);

    if (ROOT_CONFIG(NCLYR_CONFIG_CONFIG)->u.str.str && config_load_from_file(&nclyr_config, ROOT_CONFIG(NCLYR_CONFIG_CONFIG)->u.str.str) != 0) {
        printf("%s: Error reading config file '%s'\n", argv[0], ROOT_CONFIG(NCLYR_CONFIG_CONFIG)->u.str.str);
        return 1;
    }

    if (config_load_from_args(&nclyr_config, &args.parser) != 0)
        return 0;

    player_set_current(player_find(ROOT_CONFIG(NCLYR_CONFIG_PLAYER)->u.str.str));

    if (player_current() == NULL) {
        printf("%s: Error, no music player selected.\n", argv[0]);
        return 0;
    }

    iface = nclyr_iface_find(ROOT_CONFIG(NCLYR_CONFIG_INTERFACE)->u.str.str);
    if (!iface) {
        printf("%s: Error, interface '%s' does not exist.\n", argv[0], ROOT_CONFIG(NCLYR_CONFIG_INTERFACE)->u.str.str);
        return 1;
    }

    nclyr_pipes_open(&pipes);

    /* Start a thread to catch all of our signals and send them down the
     * signalfd pipe. This thread has to be started before we mask all the
     * signals on the main thread */
    signal_start_handler(pipes.sig[1]);

    /* Make sure no threads but the signal thread recieves any signals by
     * masking them on the main thread. */
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* Start 'lyr_thread', which runs glyr and querys for song information */
    lyr_thread_start(pipes.lyr[1]);

    /* Start the 'player' thread for the current player, which interfaces with
     * the actual music player */
    player_start_thread(player_current(), pipes.player[1]);

    iface->main_loop(iface, &pipes);

    DEBUG_PRINTF("Closing current player thread\n");
    player_stop_thread(player_current());

    DEBUG_PRINTF("Closing lyr thread\n");
    lyr_thread_stop();

    DEBUG_PRINTF("Closing signal thread\n");
    signal_stop_handler();

    DEBUG_PRINTF("Clearing nclyr conf\n");
    nclyr_conf_clear();

    DEBUG_PRINTF("Closing nclyr pipes\n");
    nclyr_pipes_close(&pipes);

    DEBUG_PRINTF("nclyr ending!\n");

    DEBUG_CLOSE();

    return 0;
}

