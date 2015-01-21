
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
#include "debug.h"

int main(int argc, char **argv)
{
    int pipefd[2], notifyfd[2], signalfd[2];
    struct song_info *song = NULL;
    sigset_t set;

    DEBUG_INIT();

    DEBUG_PRINTF("nclyr started!\n");

    player_set_current(player_find("mpd"));

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

    DEBUG_CLOSE();

    return 0;
}

