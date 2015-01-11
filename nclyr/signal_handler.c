
#include "common.h"

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "signal_handler.h"

static int sig_notif_fd;
static pthread_t sig_thread;

static void dummy_handler(int sig)
{
    return ;
}

static void *signal_handler(void *nothing)
{
    int exit_flag = 0, sig;
    sigset_t set;

    signal(SIGINT, dummy_handler);
    signal(SIGHUP, dummy_handler);
    signal(SIGTERM, dummy_handler);
    signal(SIGUSR1, dummy_handler);
    signal(SIGUSR2, dummy_handler);

    sigfillset(&set);

    do {
        sigwait(&set, &sig);

        if (sig == SIGUSR2)
            exit_flag = 1;
        else
            write(sig_notif_fd, &sig, sizeof(sig));

    } while (!exit_flag);

    return NULL;
}

void signal_start_handler(int pipefd)
{
    sig_notif_fd = pipefd;

    pthread_create(&sig_thread, NULL, signal_handler, NULL);
}

void signal_stop_handler(void)
{
    pthread_kill(sig_thread, SIGUSR2);
    pthread_join(sig_thread, NULL);
}


