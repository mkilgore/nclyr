
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

    pipe(pipefd);
    pipe(notifyfd);
    pipe(signalfd);

    signal_start_handler(signalfd[1]);

    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    lyr_thread_start(notifyfd[1]);
    player_setup_notification(pipefd[1]);

    tui_main_loop(signalfd[0], pipefd[0], notifyfd[0]);

    signal_stop_handler();
    player_stop_notification();
    lyr_thread_stop();

    song_clear(song);
    free(song);

    DEBUG_CLOSE();

    return 0;
}

