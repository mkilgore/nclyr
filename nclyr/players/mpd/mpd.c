
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

static struct song_info_mpd *mpd_song_to_song_info(const struct mpd_song *msong)
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

static enum player_state mpd_state_to_player_state(enum mpd_state mpd_state)
{
    return (mpd_state == MPD_STATE_PLAY)?  PLAYER_PLAYING:
           (mpd_state == MPD_STATE_PAUSE)? PLAYER_PAUSED:
                                           PLAYER_STOPPED;
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

    if (song_equal(&song->song, player->state.song)) {
        song_free(&song->song);
        return ;
    }

    song_free(player->state.song);
    player->state.song = &song->song;

    player_send_cur_song(&player->player, song_copy(player->state.song));
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

static void update_flags(struct mpd_player *player, struct mpd_status *status)
{
    int send = 0;

    if (player->state.flags.is_random != mpd_status_get_random(status)) {
        player->state.flags.is_random = mpd_status_get_random(status);
        send = 1;
    }

    if (player->state.flags.is_single != mpd_status_get_single(status)) {
        player->state.flags.is_single = mpd_status_get_single(status);
        send = 1;
    }

    if (player->state.flags.is_consume != mpd_status_get_consume(status)) {
        player->state.flags.is_consume = mpd_status_get_consume(status);
        send = 1;
    }

    if (player->state.flags.is_crossfade != mpd_status_get_crossfade(status)) {
        player->state.flags.is_crossfade = mpd_status_get_crossfade(status);
        send = 1;
    }

    if (player->state.flags.is_repeat != mpd_status_get_repeat(status)) {
        player->state.flags.is_repeat = mpd_status_get_repeat(status);
        send = 1;
    }

    if (send)
        player_send_flags(&player->player, player->state.flags);
}

static void send_initial_status(struct mpd_player *player)
{
    struct mpd_status *status;

    status = mpd_run_status(player->conn);

    if (!status)
        return ;

    player->state.seek_pos = mpd_status_get_elapsed_time(status);
    player->seek_offset = player->state.seek_pos;
    player->playing_started = time(NULL);

    player->state.volume = mpd_status_get_volume(status);
    player->state.flags.is_random = mpd_status_get_random(status);
    player->state.flags.is_single = mpd_status_get_single(status);
    player->state.flags.is_consume = mpd_status_get_consume(status);
    player->state.flags.is_crossfade = mpd_status_get_crossfade(status);
    player->state.flags.is_repeat = mpd_status_get_repeat(status);

    player->state.state = mpd_state_to_player_state(mpd_status_get_state(status));
    if (player->state.state != PLAYER_PLAYING) {
        player->seek_offset = player->state.seek_pos;
        player->playing_started = time(NULL);
    }

    player_send_seek(&player->player, player->state.seek_pos);
    player_send_volume(&player->player, player->state.volume);
    player_send_state(&player->player, player->state.state);
    player_send_flags(&player->player, player->state.flags);

    mpd_status_free(status);
}

static void update_status(struct mpd_player *player)
{
    struct mpd_status *status;

    status = mpd_run_status(player->conn);

    if (!status)
        return ;

    if (player->state.seek_pos
            != mpd_status_get_elapsed_time(status)) {
        player->state.seek_pos = mpd_status_get_elapsed_time(status);
        player->seek_offset = player->state.seek_pos;
        player->playing_started = time(NULL);

        player_send_seek(&player->player, player->state.seek_pos);
    }

    if (player->state.volume
            != mpd_status_get_volume(status)) {
        player->state.volume = mpd_status_get_volume(status);
        player_send_volume(&player->player, player->state.volume);
        DEBUG_PRINTF("Volume: %d\n", player->state.volume);
    }

    if (player->state.state
            != mpd_state_to_player_state(mpd_status_get_state(status))) {
        player->state.state = mpd_state_to_player_state(mpd_status_get_state(status));
        if (player->state.state != PLAYER_PLAYING) {
            player->seek_offset = player->state.seek_pos;
            player->playing_started = time(NULL);
        }
        player_send_state(&player->player, player->state.state);
    }

    if (player->state.song_pos
            != mpd_status_get_song_pos(status)) {
        player->state.song_pos = mpd_status_get_song_pos(status);
        player_send_song_pos(&player->player, mpd_status_get_song_pos(status));
    }

    update_flags(player, status);

    mpd_status_free(status);
}

static void update_elapsed_time(struct mpd_player *player)
{
    int new_seek_pos;
    if (player->state.state != PLAYER_PLAYING)
        return ;

    new_seek_pos = player->seek_offset + (int)difftime(time(NULL), player->playing_started);
    if (new_seek_pos != player->state.seek_pos) {
        player->state.seek_pos = new_seek_pos;
        player_send_seek(&player->player, new_seek_pos);
    }
}

static struct directory_entry *directory_add_entry(struct directory *dir)
{
    int entry = dir->entry_count++;
    dir->entries = realloc(dir->entries, sizeof(*dir->entries) * dir->entry_count);

    memset(dir->entries + entry, 0, sizeof(struct directory_entry));

    return dir->entries + entry;
}

static void send_directory(struct mpd_player *player)
{
    struct directory cwd;
    struct mpd_entity *entity;

    directory_init(&cwd);

    cwd.name = nstrdup(player->state.cwd.name);
    mpd_send_list_meta(player->conn, cwd.name);

    if (strcmp("", player->state.cwd.name) != 0) {
        struct directory_entry *entry = directory_add_entry(&cwd);

        entry->type = ENTRY_TYPE_DIR;
        entry->name = strdup("..");
    }

    while ((entity = mpd_recv_entity(player->conn))) {
        enum mpd_entity_type type = mpd_entity_get_type(entity);

        if (type != MPD_ENTITY_TYPE_SONG && type != MPD_ENTITY_TYPE_DIRECTORY)
            goto free_entity;

        struct directory_entry *entry = directory_add_entry(&cwd);

        switch (type) {
        case MPD_ENTITY_TYPE_DIRECTORY:
            entry->name = strdup(mpd_directory_get_path(mpd_entity_get_directory(entity)));
            entry->type = ENTRY_TYPE_DIR;
            break;

        case MPD_ENTITY_TYPE_SONG:
            entry->song = &mpd_song_to_song_info(mpd_entity_get_song(entity))->song;
            entry->type = ENTRY_TYPE_SONG;
            break;

        default:
            break;
        }

      free_entity:
        mpd_entity_free(entity);
    }

    DEBUG_PRINTF("Sending directory: %p\n", cwd.entries);
    player_send_directory(&player->player, &cwd);
}

static void change_directory_up(struct mpd_player *player)
{
    size_t len = strlen(player->state.cwd.name);

    /* Just mark a new end for the current string.
     *
     * Note that mpd directories don't start with '/', but '/' is still
     * considered the root. */
    if (len > 1) {
        int last_slash = 0;
        char *tmp = player->state.cwd.name;
        int i;

        for (i = 0; *tmp; tmp++, i++)
            if (*tmp == '/')
                last_slash = i;

        player->state.cwd.name[last_slash] = '\0';
    }
}

/* Waiting for mpd to be up is hacky. The issue is that 'mpd_connection_new'
 * doesn't always return quickly if there's a connection issue, instead it just
 * blocks. This becomes a problem because we're going to end-up waiting on this
 * thread when the user tries to close nclyr, meaning that we're hanging the
 * program to wait to see if we can connect to mpd just so we can kill the
 * connection and exit. Obviously, it should be better.
 *
 * The solutions is more threads (Of course...). We spawn a thread to take-care
 * of calling `mpd_connection_new`, and then we have our separate thread which
 * waits for both a response from that thread, *or* a notification that we
 * should close. We close uncleanly if we have to if we're being notified to
 * exit - It's not worth trying to clean-up cleanly when we're just going to
 * close the program anyway (And thus such clean-up will be forced if we
 * couldn't do it). */

struct mpd_conn_params {
    struct mpd_player *player;
    int pipe[2];
};

static void *mpd_connection_new_thread(void *p)
{
    struct mpd_conn_params *params = p;

    do {
        DEBUG_PRINTF("Connecting to mpd...\n");
        params->player->conn = mpd_connection_new(mpd_config[PLAYER_CONFIG_MPD_SERVER].u.str.str,
                                                  mpd_config[PLAYER_CONFIG_MPD_PORT].u.integer,
                                                  0);

        if (mpd_connection_get_error(params->player->conn) != MPD_ERROR_SUCCESS) {
            mpd_connection_free(params->player->conn);
            params->player->conn = NULL;
            /* Try a connection once every second */
            sleep(1);
        }

    } while (!params->player->conn);

    /* Flag that it worked */
    int flag = 2;
    write(params->pipe[1], &flag, sizeof(flag));

    return NULL;
}

static int wait_for_mpd_up(struct mpd_player *player)
{
    int ret = 0;
    struct mpd_conn_params params;
    struct pollfd fds[2] = { { 0 } };
    pthread_t mpd_conn_thread;

    params.player = player;
    pipe(params.pipe);

    pthread_create(&mpd_conn_thread, NULL, mpd_connection_new_thread, &params);

    fds[0].fd = player->stop_fd[0];
    fds[0].events = POLLIN;
    fds[1].fd = params.pipe[0];
    fds[1].events = POLLIN;

    do {
        poll(fds, ARRAY_SIZE(fds), -1);
        if (fds[0].revents & POLLIN) {
            ret = 1;
            goto cancel_thread;
        }

        if (fds[1].revents & POLLIN) {
            /* Connection was sucessfull */
            pthread_join(mpd_conn_thread, NULL);
            goto cleanup;
        }
    } while (!player->conn);

cancel_thread:
    pthread_cancel(mpd_conn_thread);
cleanup:
    close(params.pipe[0]);
    close(params.pipe[1]);
    return ret;
}

/* Returns 0 on normal termination.
 * Returns -1 on connection termination
 */
static int mpd_normal_loop(struct mpd_player *player)
{
    int ret = 0;
    int stop_flag = 0, idle_flag = 0;
    struct pollfd fds[3] = { { 0 } };
    enum mpd_idle idle;

    fds[0].fd = mpd_connection_get_fd(player->conn);
    fds[0].events = POLLIN;

    fds[1].fd = player->ctrl_fd[0];
    fds[1].events = POLLIN;

    fds[2].fd = player->stop_fd[0];
    fds[2].events = POLLIN;

    player_send_is_up(&player->player);

    player->state.state = PLAYER_STOPPED;
    player_send_state(&player->player, player->state.state);

    get_and_send_cur_song(player);
    get_and_send_playlist(player);
    send_initial_status(player);

    do {
        enum mpd_error err;
        int handle_idle = 0;

        if (player->state.state == PLAYER_PLAYING)
            update_elapsed_time(player);

        if (!idle_flag) {
            mpd_send_idle(player->conn);
            idle_flag = 1;
        }

        /* When we're playing, we wake up this thread every half second so we
         * can check and update the elapsed time. Doing it every half second
         * means we get a pretty good elapsed time measurement - Less jumpy
         * then a full second would be. */
        poll(fds, sizeof(fds)/sizeof(*fds), ((player->state.state) == PLAYER_PLAYING)? 500: -1);

        if (fds[2].revents & POLLIN) {
            stop_flag = 1;
            continue;
        }

        /* Basically, we can't send any new messages until we get an idle
         * message, or cancel the current idle using 'noidle'. We want to avoid
         * that if we only need to update the elapsed time though, since we can
         * do that *without* having to exit idle mode. So this checks if we're
         * going to need to do something that requires exiting idle mode first,
         * and if so sets a flag so we do. */
        if (!(fds[0].revents & POLLIN) && (fds[1].revents & POLLIN)) {
            if (fds[1].revents & POLLIN)
                DEBUG_PRINTF("Pollin on CTRL fd\n");
            DEBUG_PRINTF("Sending noidle to mpd\n");
            mpd_send_noidle(player->conn);
            handle_idle = 1;
        }

        if (fds[0].revents & POLLIN || handle_idle) {
            idle = mpd_recv_idle(player->conn, false);
            idle_flag = 0;

            DEBUG_PRINTF("Recieved idle info from mpd: %d\n", idle);

            if ((idle & MPD_IDLE_PLAYER) || (idle & MPD_IDLE_MIXER)) {
                get_and_send_cur_song(player);
                update_status(player);
            }

            if (idle & MPD_IDLE_QUEUE)
                get_and_send_playlist(player);

        }


        if (fds[1].revents & POLLIN) {
            struct player_ctrl_msg msg;
            DEBUG_PRINTF("Got control message\n");
            read(fds[1].fd, &msg, sizeof(msg));
            DEBUG_PRINTF("Read control message: %d\n", msg.type);

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
                DEBUG_PRINTF("Volume change: %d + %d\n", player->state.volume, msg.u.vol_change);
                mpd_run_set_volume(player->conn, player->state.volume + msg.u.vol_change);
                break;

            case PLAYER_CTRL_CHANGE_SONG:
                mpd_run_play_pos(player->conn, msg.u.song_pos);
                break;

            case PLAYER_CTRL_REMOVE_SONG:
                DEBUG_PRINTF("Sending remove song: %d\n", msg.u.song_pos);
                mpd_run_delete(player->conn, msg.u.song_pos);
                break;

            case PLAYER_CTRL_GET_DIRECTORY:
                send_directory(player);
                break;

            case PLAYER_CTRL_CHANGE_DIRECTORY:
                if (strcmp(msg.u.change_dir, "..") == 0) {
                    change_directory_up(player);
                    free(msg.u.change_dir);
                } else {
                    free(player->state.cwd.name);
                    player->state.cwd.name = msg.u.change_dir;
                }
                break;

            case PLAYER_CTRL_ADD_SONG:
                mpd_run_add(player->conn, msg.u.song_name);
                free(msg.u.song_name);
                break;

            case PLAYER_CTRL_SEEK:
                mpd_run_seek_pos(player->conn, player->state.song_pos, msg.u.seek_pos);
                break;

            case PLAYER_CTRL_TOGGLE_FLAGS:
                if (msg.u.flags.is_random)
                    mpd_run_random(player->conn, !player->state.flags.is_random);

                if (msg.u.flags.is_single)
                    mpd_run_single(player->conn, !player->state.flags.is_single);

                if (msg.u.flags.is_consume)
                    mpd_run_consume(player->conn, !player->state.flags.is_consume);

                if (msg.u.flags.is_crossfade)
                    mpd_run_crossfade(player->conn, !player->state.flags.is_crossfade);

                if (msg.u.flags.is_repeat)
                    mpd_run_repeat(player->conn, !player->state.flags.is_repeat);
                break;
            }

            update_status(player);
        }

        err = mpd_connection_get_error(player->conn);

        if (err != MPD_ERROR_SUCCESS) {
            DEBUG_PRINTF("Mpd error: %d\n", err);
            if (err == MPD_ERROR_CLOSED) {
                stop_flag = 1;
                ret = -1;
            }
        }
    } while (!stop_flag);

    return ret;
}

static void *mpd_thread(void *p)
{
    struct mpd_player *player = p;
    int term = 0;

    do {
        int ret;

        player_send_is_down(&player->player);

        if (wait_for_mpd_up(player)) {
            DEBUG_PRINTF("Stop recieved while trying to connect to MPD\n");
            return NULL;
        }

        DEBUG_PRINTF("Connection sucessfull\n");

        ret = mpd_normal_loop(player);

        if (ret == 0)
            term = 1;

    } while (!term);

    mpd_connection_free(player->conn);
    player_state_full_clear(&player->state);

    return NULL;
}

static void mpd_start_thread(struct player *p)
{
    struct mpd_player *player = container_of(p, struct mpd_player, player);

    pipe(player->stop_fd);
    pipe(player->ctrl_fd);

    player->state.cwd.name = strdup("");

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
        .notify_flags = 0
            | F(PN_HAS_NO_SONG)
            | F(PN_HAS_SONG)
            | F(PN_HAS_IS_UP)
            | F(PN_HAS_IS_DOWN)
            | F(PN_HAS_STATE)
            | F(PN_HAS_SEEK)
            | F(PN_HAS_VOLUME)
            | F(PN_HAS_PLAYLIST)
            | F(PN_HAS_SONG_POS)
            | F(PN_HAS_DIRECTORY)
            | F(PN_HAS_FLAGS)
            ,
        .start_thread = mpd_start_thread,
        .stop_thread = mpd_stop_thread,
        .ctrls = {
            .ctrl = mpd_send_ctrl_msg,
            .has_ctrl_flag = 0
                | F(PC_HAS_PAUSE)
                | F(PC_HAS_TOGGLE_PAUSE)
                | F(PC_HAS_PLAY)
                | F(PC_HAS_NEXT)
                | F(PC_HAS_PREV)
                | F(PC_HAS_SEEK)
                | F(PC_HAS_SET_VOLUME)
                | F(PC_HAS_CHANGE_VOLUME)
                | F(PC_HAS_CHANGE_SONG)
                | F(PC_HAS_MOVE_SONG)
                | F(PC_HAS_ADD_SONG)
                | F(PC_HAS_GET_DIRECTORY)
                | F(PC_HAS_CHANGE_DIRECTORY)
                | F(PC_HAS_TOGGLE_FLAGS)
                ,
        }
    },
    .state = PLAYER_STATE_FULL_INIT(mpd_player.state)
};

