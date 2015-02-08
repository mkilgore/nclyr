
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <pthread.h>
#include <mpd/client.h>

#include "a_sprintf.h"
#include "nstrdup.h"
#include "song.h"
#include "playlist.h"
#include "player.h"
#include "song_info_mpd.h"
#include "mpd.h"
#include "debug.h"

static struct song_info_mpd *mpd_song_to_song_info(struct mpd_song *msong)
{
    struct song_info_mpd *isong = malloc(sizeof(*isong));
    song_info_mpd_init(isong);

    isong->song.name = nstrdup(mpd_song_get_uri(msong));
    isong->song.tag.artist = nstrdup(mpd_song_get_tag(msong, MPD_TAG_ARTIST, 0));
    isong->song.tag.title = nstrdup(mpd_song_get_tag(msong, MPD_TAG_TITLE, 0));
    isong->song.tag.album = nstrdup(mpd_song_get_tag(msong, MPD_TAG_ALBUM, 0));
    isong->song.duration = mpd_song_get_duration(msong);

    return isong;
}

static struct song_info_mpd *get_cur_song(struct mpd_connection *conn)
{
    struct song_info_mpd *song;
    struct mpd_song *msong;

    DEBUG_PRINTF("Asking mpd for current song\n");

    mpd_send_current_song(conn);

    msong = mpd_recv_song(conn);

    DEBUG_PRINTF("Recieved song\n");

    mpd_response_finish(conn);

    song = mpd_song_to_song_info(msong);
    mpd_song_free(msong);

    return song;
}

static void get_and_send_cur_song(struct mpd_player *player)
{
    struct song_info_mpd *song;

    song = get_cur_song(player->conn);

    if (song_equal(&song->song, &player->cur_song->song)) {
        song_free(&song->song);
        return ;
    }

    song_free(&player->cur_song->song);
    player->cur_song = song;

    player_send_cur_song(&player->player, song_copy(&player->cur_song->song));
    return ;
}

static void get_cur_playlist(struct mpd_player *player, struct playlist *playlist)
{
    struct mpd_song *msong;
    mpd_send_list_queue_meta(player->conn);

    playlist->song_count = 0;
    playlist->songs = NULL;

    while ((msong = mpd_recv_song(player->conn)) != NULL) {
        playlist->song_count++;
        playlist->songs = realloc(playlist->songs, playlist->song_count * sizeof(*playlist->songs));
        playlist->songs[playlist->song_count - 1] = &mpd_song_to_song_info(msong)->song;
        mpd_song_free(msong);
    }
}

static void get_and_send_playlist(struct mpd_player *player)
{
    struct playlist playlist;

    get_cur_playlist(player, &playlist);

    player_send_playlist(&player->player, &playlist);
}

static void update_status(struct mpd_player *player)
{
    struct mpd_status *status;

    status = mpd_run_status(player->conn);

    if (mpd_status_get_elapsed_time(player->cur_status)
            != mpd_status_get_elapsed_time(status)) {
        player_send_seek(&player->player, mpd_status_get_elapsed_time(status));
        DEBUG_PRINTF("Seek: %d\n", mpd_status_get_elapsed_time(status));
    }

    if (mpd_status_get_volume(player->cur_status)
            != mpd_status_get_volume(status)) {
        player_send_volume(&player->player, mpd_status_get_volume(status));
        DEBUG_PRINTF("Volume: %d\n", mpd_status_get_volume(status));
    }

    if (mpd_status_get_state(player->cur_status)
            != mpd_status_get_state(status)) {
        enum mpd_state mpd_state;
        enum player_state state;

        mpd_state = mpd_status_get_state(status);
        state = (mpd_state == MPD_STATE_PLAY)? PLAYER_PLAYING:
                (mpd_state == MPD_STATE_PAUSE)? PLAYER_PAUSED:
                (mpd_state == MPD_STATE_STOP)? PLAYER_STOPPED:
                PLAYER_STOPPED;

        player_send_state(&player->player, state);
    }

    if (mpd_status_get_song_pos(player->cur_status)
            != mpd_status_get_song_pos(status)) {
        player_send_song_pos(&player->player, mpd_status_get_song_pos(status));
    }

    mpd_status_free(player->cur_status);
    player->cur_status = status;
}

static void *mpd_thread(void *p)
{
    int stop_flag = 0;
    struct mpd_player *player = p;
    struct pollfd fds[3] = { { 0 } };
    enum mpd_idle idle;

    DEBUG_PRINTF("Connecting to mpd...\n");
    player->conn = mpd_connection_new("127.0.0.1", 6600, 0);

    if (mpd_connection_get_error(player->conn) != MPD_ERROR_SUCCESS) {
        mpd_connection_free(player->conn);
        return NULL;
    }

    DEBUG_PRINTF("Connection sucessfull\n");

    player_send_is_up(&player->player);

    get_and_send_cur_song(player);
    get_and_send_playlist(player);
    player->cur_status = mpd_status_begin();

    fds[0].fd = mpd_connection_get_fd(player->conn);
    fds[0].events = POLLIN;

    fds[1].fd = player->ctrl_fd[0];
    fds[1].events = POLLIN;

    fds[2].fd = player->stop_fd[0];
    fds[2].events = POLLIN;

    do {
        int handle_idle = 0;

        if (player->conn)
            update_status(player);

        mpd_send_idle(player->conn);
        poll(fds, sizeof(fds)/sizeof(*fds), (mpd_status_get_state(player->cur_status) == MPD_STATE_PLAY)? 900: -1);

        if (fds[2].revents & POLLIN) {
            stop_flag = 1;
            continue;
        }

        if (!(fds[0].revents & POLLIN)) {
            mpd_send_noidle(player->conn);
            handle_idle = 1;
        }

        if (fds[0].revents & POLLIN || handle_idle) {
            DEBUG_PRINTF("Recieved idle info from mpd!\n");
            idle = mpd_recv_idle(player->conn, false);
            if (idle & MPD_IDLE_PLAYER)
                get_and_send_cur_song(player);

            if (idle & MPD_IDLE_QUEUE)
                get_and_send_playlist(player);
        }

        if (fds[1].revents & POLLIN) {
            struct player_ctrl_msg msg;
            DEBUG_PRINTF("Got control message\n");
            read(fds[1].fd, &msg, sizeof(msg));
            DEBUG_PRINTF("Read control message\n");

            switch (msg.type) {
            case PLAYER_CTRL_PLAY:
                mpd_run_play(player->conn);
                break;

            case PLAYER_CTRL_PAUSE:
                mpd_run_pause(player->conn, msg.u.pause);
                break;

            case PLAYER_CTRL_TOGGLE_PAUSE:
                mpd_run_toggle_pause(player->conn);
                break;

            case PLAYER_CTRL_NEXT:
                mpd_run_next(player->conn);
                break;

            case PLAYER_CTRL_PREV:
                mpd_run_previous(player->conn);
                break;

            case PLAYER_CTRL_SET_VOLUME:
                mpd_run_set_volume(player->conn, msg.u.volume);
                break;

            case PLAYER_CTRL_CHANGE_VOLUME:
                mpd_run_set_volume(player->conn, mpd_status_get_volume(player->cur_status) + msg.u.vol_change);
                break;

            case PLAYER_CTRL_CHANGE_SONG:
                mpd_run_play_pos(player->conn, msg.u.song_pos);
                break;

            case PLAYER_CTRL_SEEK:
            case PLAYER_CTRL_SHUFFLE:
                break;

            }
        }

    } while (!stop_flag);

    mpd_connection_free(player->conn);
    mpd_status_free(player->cur_status);
    song_free(&player->cur_song->song);

    return NULL;
}

static void mpd_start_thread(struct player *p)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);

    pipe(player->stop_fd);
    pipe(player->ctrl_fd);

    pthread_create(&player->mpd_thread, NULL, mpd_thread, player);
    return ;
}

static void mpd_stop_thread(struct player *p)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);
    int tmp = 0;

    write(player->stop_fd[1], &tmp, sizeof(tmp));
    DEBUG_PRINTF("Joining mpd thread...\n");
    pthread_join(player->mpd_thread, NULL);
    DEBUG_PRINTF("mpd thread closed.\n");

    close(player->stop_fd[0]);
    close(player->stop_fd[1]);

    close(player->ctrl_fd[0]);
    close(player->ctrl_fd[1]);

    return ;
}

static void mpd_send_ctrl_msg (struct player *p, const struct player_ctrl_msg *msg)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);
    write(player->ctrl_fd[1], msg, sizeof(*msg));
}

struct mpd_player mpd_player = {
    .player = {
        .name = "mpd",
        .start_thread = mpd_start_thread,
        .stop_thread = mpd_stop_thread,
        .ctrls = {
            .ctrl = mpd_send_ctrl_msg,
            .has_pause = 1,
            .has_toggle_pause = 1,
            .has_play = 1,
            .has_next = 1,
            .has_prev = 1,
            .has_seek = 0,
            .has_shuffle = 0,
            .has_set_volume = 1,
            .has_change_volume = 1
        }
    }
};

