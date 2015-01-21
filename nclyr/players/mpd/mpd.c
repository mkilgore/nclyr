
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <pthread.h>
#include <mpd/client.h>

#include "player.h"
#include "song.h"
#include "mpd.h"
#include "debug.h"

static void mpd_song_to_song_info(struct mpd_song *msong, struct song_info *isong)
{
    isong->artist = strdup(mpd_song_get_tag(msong, MPD_TAG_ARTIST, 0));
    isong->title = strdup(mpd_song_get_tag(msong, MPD_TAG_TITLE, 0));
    isong->album = strdup(mpd_song_get_tag(msong, MPD_TAG_ALBUM, 0));
}

static void get_cur_song(struct mpd_connection *conn, struct song_info *song)
{
    struct mpd_song *msong;

    mpd_command_list_begin(conn, 1);
    mpd_send_current_song(conn);
    mpd_command_list_end(conn);

    msong = mpd_recv_song(conn);

    mpd_response_finish(conn);

    mpd_song_to_song_info(msong, song);
    mpd_song_free(msong);
}

static void get_and_send_cur_song(struct mpd_connection *conn, int notify_fd)
{
    struct song_info song;
    struct player_notification notif;

    get_cur_song(conn, &song);
    memset(&notif, 0, sizeof(notif));
    notif.type = PLAYER_SONG;
    notif.u.song = song;
    write(notify_fd, &notif, sizeof(notif));

    return ;
}

static void *mpd_thread(void *p)
{
    int stop_flag = 0;
    struct mpd_player *player = p;
    struct pollfd fds[2];
    enum mpd_idle idle;
    struct player_notification notif;

    player->conn = mpd_connection_new("127.0.0.1", 6600, 0);

    if (!player->conn)
        return NULL;

    memset(&notif, 0, sizeof(notif));
    notif.type = PLAYER_IS_UP;
    write(player->notify_fd, &notif, sizeof(notif));

    get_and_send_cur_song(player->conn, player->notify_fd);

    fds[0].fd = mpd_connection_get_fd(player->conn);
    fds[0].events = POLLIN;

    fds[1].fd = player->stop_fd[0];
    fds[1].events = POLLIN;

    do {
        mpd_send_idle(player->conn);
        poll(fds, 2, -1);

        if (fds[1].revents & POLLIN) {
            stop_flag = 1;
        }

        if (fds[0].revents & POLLIN) {
            DEBUG_PRINTF("Recieved info from mpd!\n");
            idle = mpd_recv_idle(player->conn, false);
            if (idle & MPD_IDLE_PLAYER)
                get_and_send_cur_song(player->conn, player->notify_fd);
        }
    } while (!stop_flag);

    mpd_connection_free(player->conn);

    return NULL;
}

static void mpd_setup_notification(struct player *p, int pipfd)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);

    player->notify_fd = pipfd;
    pipe(player->stop_fd);

    pthread_create(&player->mpd_thread, NULL, mpd_thread, player);
    return ;
}

static void mpd_stop_notification(struct player *p)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);
    int tmp = 0;

    write(player->stop_fd[1], &tmp, sizeof(tmp));
    pthread_join(player->mpd_thread, NULL);

    return ;
}

struct mpd_player mpd_player = {
    .player = {
        .name = "mpd",
        .start_monitor = mpd_setup_notification,
        .stop_monitor = mpd_stop_notification
    }
};

