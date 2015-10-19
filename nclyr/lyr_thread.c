
#include "common.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <pthread.h>
#include <glyr/glyr.h>

#include "player.h"
#include "lyr_thread.h"
#include "debug.h"

struct lyr_thread {
    struct song_info *song;
    GLYR_DATA_TYPE intended_type;
    pthread_t thread;

    GlyrQuery q;
    GLYR_ERROR err;
    GlyrMemCache *head;

    int pipefd;
};

static void lyr_thread_init(struct lyr_thread *song_data)
{
    memset(song_data, 0, sizeof(*song_data));
    glyr_query_init(&song_data->q);
}

static void lyr_thread_clear(struct lyr_thread *song_data)
{
    if (song_data->head)
        glyr_free_list(song_data->head);

    glyr_query_destroy(&song_data->q);
}

static void *handle_query(void *s)
{
    struct lyr_thread *song_data = (struct lyr_thread *)s;
    int len;

    song_data->head = glyr_get(&song_data->q, &song_data->err, &len);

    write(song_data->pipefd, &song_data, sizeof(song_data));
    return NULL;
}

static pthread_t song_queue_thread;
static int song_notify_fd, stop_fd[2], add_song_fd[2], thread_exit[2];

static void start_lyr_thread(const struct song_info *song, int thread_fd, const enum lyr_data_type type)
{
    struct lyr_thread *song_data;

    song_data = malloc(sizeof(*song_data));

    lyr_thread_init(song_data);

    song_data->song = song_copy(song);
    song_data->pipefd = thread_fd;
    song_data->intended_type = type;

    /* Note - glyr_opt_* will strdup these strings, so 'song' can be
     * deleted even if the query is still going on.
     *
     * This is important as if we're going to replace the current song
     * with a new one via lookup, we can delete the old song's info
     * whenever we want. */
    glyr_opt_title(&song_data->q, song->tag.title);
    glyr_opt_artist(&song_data->q, song->tag.artist);
    glyr_opt_album(&song_data->q, song->tag.album);

    switch (type) {
    case LYR_ARTIST_BIO:
        glyr_opt_type(&song_data->q, GLYR_GET_ARTISTBIO);
        song_data->intended_type = GLYR_TYPE_ARTIST_BIO;
        break;
    case LYR_SIMILAR_SONG:
        glyr_opt_type(&song_data->q, GLYR_GET_SIMILAR_SONGS);
        song_data->intended_type = GLYR_TYPE_SIMILAR_SONG;
        break;
    case LYR_SIMILAR_ARTIST:
        glyr_opt_type(&song_data->q, GLYR_GET_SIMILAR_ARTISTS);
        song_data->intended_type = GLYR_TYPE_SIMILAR_ARTIST;
        break;
    case LYR_COVERART:
        glyr_opt_type(&song_data->q, GLYR_GET_COVERART);
        song_data->intended_type = GLYR_TYPE_COVERART;
        break;
    case LYR_LYRICS:
        glyr_opt_type(&song_data->q, GLYR_GET_LYRICS);
        song_data->intended_type = GLYR_TYPE_LYRICS;
        break;
    case LYR_DATA_TYPE_COUNT:
        break;
    }

    pthread_create(&song_data->thread, NULL, handle_query, song_data);

}

static void start_lyr_thread_together(const struct song_info *song, const enum lyr_data_type *type)
{
    const enum lyr_data_type *t;
    for (t = type; *t != LYR_DATA_TYPE_COUNT; t++)
        start_lyr_thread(song, thread_exit[1], *t);
}

static void process_glyr_result(struct lyr_thread *song_thr)
{
    struct lyr_thread_notify song_notify;
    GlyrMemCache *cur;

    memset(&song_notify, 0, sizeof(song_notify));
    song_notify.song = song_thr->song;

    switch (song_thr->intended_type) {
    case GLYR_TYPE_ARTIST_BIO:
        song_notify.type = LYR_ARTIST_BIO;
        break;

    case GLYR_TYPE_SIMILAR_SONG:
        song_notify.type = LYR_SIMILAR_SONG;
        break;

    case GLYR_TYPE_SIMILAR_ARTIST:
        song_notify.type = LYR_SIMILAR_ARTIST;
        break;

    case GLYR_TYPE_COVERART:
        song_notify.type = LYR_COVERART;
        break;

    case GLYR_TYPE_LYRICS:
        song_notify.type = LYR_LYRICS;
        break;

    default:
        break;
    }

    for (cur = song_thr->head; cur != NULL; cur = cur->next) {
        if (song_thr->intended_type != cur->type)
            continue;

        switch (cur->type) {
        case GLYR_TYPE_ARTIST_BIO:
            DEBUG_PRINTF("Got artist bio\n");
            song_notify.u.bio = strdup(cur->data);
            song_notify.was_recieved = 1;
            goto found_glyr_data;

        case GLYR_TYPE_SIMILAR_SONG:
            DEBUG_PRINTF("Got similar song\n");
            song_notify.u.similar_songs = strdup(cur->data);
            song_notify.was_recieved = 1;
            goto found_glyr_data;

        case GLYR_TYPE_SIMILAR_ARTIST:
            DEBUG_PRINTF("Got similar artist\n");
            song_notify.u.similar_artist = strdup(cur->data);
            song_notify.was_recieved = 1;
            goto found_glyr_data;

        case GLYR_TYPE_COVERART:
            DEBUG_PRINTF("Got coverart\n");
            song_notify.u.img.format = strdup(cur->img_format);
            song_notify.u.img.data = malloc(cur->size);
            memcpy(song_notify.u.img.data, cur->data, cur->size);
            song_notify.u.img.size = cur->size;
            song_notify.was_recieved = 1;
            goto found_glyr_data;

        case GLYR_TYPE_LYRICS:
            DEBUG_PRINTF("Got lyrics\n");
            song_notify.u.lyrics = strdup(cur->data);
            song_notify.was_recieved = 1;
            goto found_glyr_data;

        default:
            break;
        }

    }

  found_glyr_data:
    write(song_notify_fd, &song_notify, sizeof(song_notify));
}

static void *lyr_thread(void *nothing)
{
    int exit_flag = 0;
    nfds_t nfds = 2;
    struct pollfd fds[nfds];

    memset(fds, 0, sizeof(fds));

    fds[0].fd = stop_fd[0];
    fds[0].events = POLLIN;

    fds[1].fd = thread_exit[0];
    fds[1].events = POLLIN;

    do {
        poll(fds, nfds, -1);

        if (fds[0].revents & POLLIN) {
            exit_flag = 1;
            continue;
        }

        if (fds[1].revents & POLLIN) {
            struct lyr_thread *song_thr;
            read(fds[1].fd, &song_thr, sizeof(song_thr));

            DEBUG_PRINTF("Recieved GLYR result: %d\n", song_thr->intended_type);

            process_glyr_result(song_thr);

            pthread_join(song_thr->thread, NULL);

            lyr_thread_clear(song_thr);
            free(song_thr);
        }


    } while (!exit_flag);

    return NULL;
}

void lyr_thread_song_lookup(const struct song_info *song, const enum lyr_data_type *list)
{
    start_lyr_thread_together(song, list);
}

void lyr_thread_start(int notify_fd)
{
    song_notify_fd = notify_fd;

    glyr_init();

    pipe(stop_fd);
    pipe(add_song_fd);
    pipe(thread_exit);

    pthread_create(&song_queue_thread, NULL, lyr_thread, NULL);
}

void lyr_thread_stop(void)
{
    int tmp = 0;
    write(stop_fd[1], &tmp, sizeof(tmp));
    DEBUG_PRINTF("Joining lyr thread\n");
    pthread_join(song_queue_thread, NULL);

    DEBUG_PRINTF("Closing glyr thread\n");
    glyr_cleanup();

    DEBUG_PRINTF("glyr closed\n");

    close(stop_fd[0]);
    close(stop_fd[1]);
    close(add_song_fd[0]);
    close(add_song_fd[1]);
    close(thread_exit[0]);
    close(thread_exit[1]);
}

void lyr_thread_notify_clear(struct lyr_thread_notify *song_notif)
{
    song_free(song_notif->song);
    switch (song_notif->type) {
    case LYR_ARTIST_BIO:
        free(song_notif->u.bio);
        break;
    case LYR_SIMILAR_SONG:
        free(song_notif->u.similar_songs);
        break;
    case LYR_SIMILAR_ARTIST:
        free(song_notif->u.similar_artist);
        break;
    case LYR_COVERART:
        free(song_notif->u.img.format);
        free(song_notif->u.img.data);
        break;
    case LYR_LYRICS:
        free(song_notif->u.lyrics);
        break;
    case LYR_DATA_TYPE_COUNT:
        break;
    }
}

