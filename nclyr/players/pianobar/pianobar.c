
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

#include "a_sprintf.h"
#include "filename.h"
#include "player.h"
#include "song.h"
#include "pianobar.h"
#include "debug.h"

static struct song_info *pianobar_get_cur_song(const char *nowplaying)
{
    struct song_info *sng;
    char buffer[500];
    char *cur, *start;
    const char *title = NULL;
    const char *artist = NULL;
    const char *album = NULL;
    int fd;

    fd = open(nowplaying, O_RDONLY | O_NONBLOCK);

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
        } else if (*cur == '\n') {
            *cur = '\0';
        }
    }

    album = start;

    if (!artist || !title)
        return NULL;

    sng = malloc(sizeof(*sng));

    song_init(sng);

    sng->tag.artist = strdup(artist);
    sng->tag.title = strdup(title);
    sng->tag.album = strdup(album);

    a_sprintf(&sng->name, "%s - %s", sng->tag.artist, sng->tag.title);

    return sng;
}

static void *pianobar_inotify_thread(void *player)
{
    struct pianobar_player *pianobar = player;
    char buffer[2048];
    int inotify, exit_flag = 0;
    struct song_info *song;
    struct pollfd fds[2];
    char *nowplaying;
    inotify = inotify_init1(O_NONBLOCK);

    nowplaying = filename_get(pianobar_config[PLAYER_CONFIG_PIANOBAR_NOWPLAYING].u.str.str);

    DEBUG_PRINTF("Nowplaying: %s\n", nowplaying);

    inotify_add_watch(inotify, nowplaying, IN_MODIFY);

    player_send_is_up(&pianobar->player);

    song = pianobar_get_cur_song(nowplaying);

    if (song) {
        player_send_state(&pianobar->player, PLAYER_PLAYING);
        pianobar->current_song = song;
        player_send_cur_song(&pianobar->player, song_copy(song));
    } else {
        player_send_no_song(&pianobar->player);
    }

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

            song = pianobar_get_cur_song(nowplaying);

            if (!song)
                continue ;

            if (!song->tag.artist || !song->tag.title || !song->tag.album) {
                song_free(song);
                continue ;
            }


            if (song_equal(song, pianobar->current_song)) {
                song_free(song);
                continue ;
            }

            DEBUG_PRINTF("New song: %s by %s on %s\n", song->tag.title, song->tag.artist, song->tag.album);

            song_free(pianobar->current_song);
            pianobar->current_song = song;

            player_send_state(&pianobar->player, PLAYER_PLAYING);
            player_send_cur_song(&pianobar->player, song_copy(pianobar->current_song));
        }
    } while (!exit_flag);

    song_free(pianobar->current_song);

    close(inotify);
    free(nowplaying);
    return NULL;
}

static void pianobar_start_thread(struct player *player)
{
    struct pianobar_player *pianobar = container_of(player, struct pianobar_player, player);

    memset(&pianobar->notif_thread, 0, sizeof(pianobar->notif_thread));

    pipe(pianobar->stop_pipe);

    pthread_create(&pianobar->notif_thread, NULL, pianobar_inotify_thread, pianobar);

    return ;
}

static void pianobar_stop_thread(struct player *player)
{
    struct pianobar_player *pianobar = container_of(player, struct pianobar_player, player);
    int tmp = 2;

    write(pianobar->stop_pipe[1], &tmp, sizeof(tmp));
    pthread_join(pianobar->notif_thread, NULL);
}

struct pianobar_player pianobar_player = {
    .player = {
        .name = "pianobar",
        .start_thread = pianobar_start_thread,
        .stop_thread = pianobar_stop_thread,
        .player_windows = (const struct nclyr_win *[]) { NULL }
    },
    .stop_pipe = { 0, 0},
};

