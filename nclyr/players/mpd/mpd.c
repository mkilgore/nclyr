
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/inotify.h>
#include <pthread.h>

#include "player.h"
#include "song.h"
#include "mpd.h"
#include "debug.h"

static void mpd_setup_notification(int pipfd)
{
    return ;
}

static void mpd_stop_notification(void)
{

}

struct mpd_player mpd_player = {
    .player = {
        "mpd",
        mpd_setup_notification,
        mpd_stop_notification
    }
};

