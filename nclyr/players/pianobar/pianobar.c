
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
#include "pianobar.h"
#include "debug.h"

const static char *piano_bar_nowplaying = "/home/dsman195276/.config/pianobar/nowplaying";

static void pianobar_get_cur_song(struct song_info *sng)
{
    char buffer[500];
    char *cur, *start;
    const char *title = NULL;
    const char *artist = NULL;
    const char *album = NULL;
    int fd;

    fd = open(piano_bar_nowplaying, O_RDONLY | O_NONBLOCK);

    memset(buffer, 0, sizeof(buffer));
    read(fd, buffer, sizeof(buffer));

    for (cur = start = buffer; *cur; cur++) {
        if (*cur == '|') {
            *cur = '\0';
            if (!artist) {
                artist = start;
            } else if (!title) {
                title = start;
            }
            start = cur + 1;
        }
    }

    album = start;

    if (!artist || !title)
        return ;

    song_init(sng);

    sng->artist = strdup(artist);
    sng->title = strdup(title);
    sng->album = strdup(album);

    return ;
}

static void *pianobar_inotify_thread(void *player)
{
    struct pianobar_player *pianobar = player;
    char buffer[2048];
    int inotify, exit_flag = 0;
    struct player_notification notif;
    struct song_info song;
    struct pollfd fds[2];
    inotify = inotify_init1(O_NONBLOCK);

    inotify_add_watch(inotify, piano_bar_nowplaying, IN_MODIFY);

    memset(&notif, 0, sizeof(struct player_notification));
    notif.type = PLAYER_NO_SONG;
    write(pianobar->output_pipe, &notif, sizeof(notif));

    fds[0].fd = inotify;
    fds[0].events = POLLIN;

    fds[1].fd = pianobar->stop_pipe[0];
    fds[1].events = POLLIN;

    do {
        poll(fds, sizeof(fds)/sizeof(fds[0]), -1);

        if (fds[1].revents & POLLIN)
            exit_flag = 1;

        if (fds[0].revents & POLLIN) {
            while (read(inotify, buffer, sizeof(buffer)) != -1)
                ;

            memset(&song, 0, sizeof(struct song_info));
            pianobar_get_cur_song(&song);

            if (!song.artist || !song.title || !song.album)
                continue ;

            if (song_equal(&song, &pianobar->current_song)) {
                song_clear(&song);
                continue ;
            }

            DEBUG_PRINTF("New song: %s by %s on %s\n", song.title, song.artist, song.album);

            song_clear(&pianobar->current_song);
            pianobar->current_song = song;

            memset(&notif, 0, sizeof(struct player_notification));
            notif.type = PLAYER_SONG;
            song_copy(&notif.u.song, &pianobar->current_song);
            write(pianobar->output_pipe, &notif, sizeof(notif));
        }

    } while (!exit_flag);

    close(inotify);
    return NULL;
}

static void pianobar_setup_notification(struct player *player, int pipfd)
{
    struct pianobar_player *pianobar = container_of(player, struct pianobar_player, player);

    memset(&pianobar->notif_thread, 0, sizeof(pianobar->notif_thread));

    pipe(pianobar->stop_pipe);
    pianobar->output_pipe = pipfd;

    pthread_create(&pianobar->notif_thread, NULL, pianobar_inotify_thread, pianobar);

    return ;
}

static void pianobar_stop_notification(struct player *player)
{
    struct pianobar_player *pianobar = container_of(player, struct pianobar_player, player);
    int tmp = 2;

    write(pianobar->stop_pipe[1], &tmp, sizeof(tmp));
    pthread_join(pianobar->notif_thread, NULL);
}

struct pianobar_player pianobar_player = {
    .player = {
        "pianobar",
        pianobar_setup_notification,
        pianobar_stop_notification
    },
    .output_pipe = 0,
    .stop_pipe = { 0, 0},
};

