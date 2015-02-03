
#include "common.h"

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "signal_handler.h"
#include "debug.h"

static int sig_notif_fd;
static pthread_t sig_thread;

static void *signal_handler(void *nothing)
{
    int sig;
    sigset_t set;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    sigfillset(&set);

    do {
        sigwait(&set, &sig);

        DEBUG_PRINTF("Signal thread got: %d\n", sig);

        write(sig_notif_fd, &sig, sizeof(sig));
    } while (1);

    return NULL;
}

void signal_start_handler(int pipefd)
{
    sig_notif_fd = pipefd;

    pthread_create(&sig_thread, NULL, signal_handler, NULL);
}

void signal_stop_handler(void)
{
    DEBUG_PRINTF("Sending cancel to signal thread\n");
    pthread_cancel(sig_thread);
    DEBUG_PRINTF("Joining signal thread\n");
    pthread_join(sig_thread, NULL);
    DEBUG_PRINTF("Signal thread closed\n");
}


