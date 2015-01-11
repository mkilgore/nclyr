
#include "common.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <pthread.h>
#include <glyr/glyr.h>

#include "player.h"

void song_init(struct song_info *sng)
{
    memset(sng, 0, sizeof(*sng));
}

void song_clear(struct song_info *sng)
{
    if (!sng)
        return ;
    free(sng->title);
    free(sng->artist);
    free(sng->album);
}

void song_copy(struct song_info *dest, struct song_info *src)
{
    dest->title = strdup(src->title);
    dest->artist = strdup(src->artist);
    dest->album = strdup(src->album);
}

int song_equal(struct song_info *src1, struct song_info *src2)
{
    if (!src1 || !src2)
        return 0;
    if (!src1->title || !src2->title || !!strcmp(src1->title, src2->title))
        return 0;
    if (!src1->artist || !src2->artist || !!strcmp(src1->artist, src2->artist))
        return 0;
    if (!src1->album || !src2->album || !!strcmp(src1->album, src2->album))
        return 0;
    return 1;
}

struct song_thread {
    const struct song_info *song;
    pthread_t thread;

    GlyrQuery q;
    GLYR_ERROR err;
    GlyrMemCache *head;

    int pipefd;
};

static void song_thread_init(struct song_thread *song_data)
{
    memset(song_data, 0, sizeof(*song_data));
    glyr_query_init(&song_data->q);
}

static void song_thread_clear(struct song_thread *song_data)
{
    if (song_data->head)
        glyr_free_list(song_data->head);

    glyr_query_destroy(&song_data->q);
}

static void *handle_query(void *s)
{
    struct song_thread *song_data = (struct song_thread *)s;
    int len;

    song_data->head = glyr_get(&song_data->q, &song_data->err, &len);

    write(song_data->pipefd, &song_data, sizeof(song_data));
    return NULL;
}

static pthread_t song_queue_thread;
static int song_notify_fd, stop_fd[2], add_song_fd[2], thread_exit[2];

static void start_song_thread(const struct song_info *song, int thread_fd, const enum song_data_type type)
{
    struct song_thread *song_data;

    song_data = malloc(sizeof(*song_data));

    song_thread_init(song_data);

    song_data->song = song;
    song_data->pipefd = thread_fd;

    /* Note - glyr_opt_* will strdup these strings, so 'song' can be
     * deleted even if the query is still going on.
     *
     * This is important as if we're going to replace the current song
     * with a new one via lookup, we can delete the old song's info
     * whenever we want. */
    glyr_opt_title(&song_data->q, song->title);
    glyr_opt_artist(&song_data->q, song->artist);
    glyr_opt_album(&song_data->q, song->album);

    switch (type) {
    case SONG_ARTIST_BIO:
        glyr_opt_type(&song_data->q, GLYR_GET_ARTISTBIO);
        break;
    case SONG_SIMILAR_SONG:
        glyr_opt_type(&song_data->q, GLYR_GET_SIMILAR_SONGS);
        break;
    case SONG_SIMILAR_ARTIST:
        glyr_opt_type(&song_data->q, GLYR_GET_SIMILAR_ARTISTS);
        break;
    case SONG_COVERART:
        glyr_opt_type(&song_data->q, GLYR_GET_COVERART);
        break;
    case SONG_LYRICS:
        glyr_opt_type(&song_data->q, GLYR_GET_LYRICS);
        break;
    case SONG_DATA_TYPE_COUNT:
        break;
    }

    pthread_create(&song_data->thread, NULL, handle_query, song_data);

}

static void start_song_thread_together(const struct song_info *song, const enum song_data_type *type)
{
    const enum song_data_type *t;
    for (t = type; *t != -1; t++)
        start_song_thread(song, thread_exit[1], *t);
}

static void *song_thread(void *nothing)
{
    int exit_flag = 0;
    nfds_t nfds = 2;
    struct song_thread_notify song_notify;
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
            struct song_thread *song_thr;
            GlyrMemCache *cur;
            read(fds[1].fd, &song_thr, sizeof(song_thr));

            memset(&song_notify, 0, sizeof(song_notify));
            song_notify.song = song_thr->song;
            for (cur = song_thr->head; cur != NULL; cur = cur->next) {
                song_notify.type = -1;

                switch (cur->type) {
                case GLYR_TYPE_ARTIST_BIO:
                    song_notify.type = SONG_ARTIST_BIO;
                    song_notify.u.bio = strdup(cur->data);
                    break;
                case GLYR_TYPE_SIMILAR_SONG:
                    song_notify.type = SONG_SIMILAR_SONG;
                    song_notify.u.similar_songs = strdup(cur->data);
                    break;
                case GLYR_TYPE_SIMILAR_ARTIST:
                    song_notify.type = SONG_SIMILAR_ARTIST;
                    song_notify.u.similar_artist = strdup(cur->data);
                    break;
                case GLYR_TYPE_COVERART:
                    song_notify.type = SONG_COVERART;
                    song_notify.u.img.format = strdup(cur->img_format);
                    song_notify.u.img.data = malloc(cur->size);
                    memcpy(song_notify.u.img.data, cur->data, cur->size);
                    song_notify.u.img.size = cur->size;
                    break;
                case GLYR_TYPE_LYRICS:
                    song_notify.type = SONG_LYRICS;
                    song_notify.u.lyrics = strdup(cur->data);
                    break;
                default:
                    break;
                }

                if (song_notify.type != -1)
                    write(song_notify_fd, &song_notify, sizeof(song_notify));
            }

            pthread_join(song_thr->thread, NULL);

            song_thread_clear(song_thr);
            free(song_thr);
        }


    } while (!exit_flag);

    return NULL;
}

void song_thread_song_lookup(const struct song_info *song, const enum song_data_type *list)
{
    start_song_thread_together(song, list);
}

void song_thread_start(int notify_fd)
{
    song_notify_fd = notify_fd;

    glyr_init();

    pipe(stop_fd);
    pipe(add_song_fd);
    pipe(thread_exit);

    pthread_create(&song_queue_thread, NULL, song_thread, NULL);
}

void song_thread_stop(void)
{
    int tmp = 0;
    write(stop_fd[1], &tmp, sizeof(tmp));
    pthread_join(song_queue_thread, NULL);

    glyr_cleanup();

    close(stop_fd[0]);
    close(stop_fd[1]);
    close(add_song_fd[0]);
    close(add_song_fd[1]);
    close(thread_exit[0]);
    close(thread_exit[1]);
}

void song_thread_notify_clear(struct song_thread_notify *song_notif)
{
    switch (song_notif->type) {
    case SONG_ARTIST_BIO:
        free(song_notif->u.bio);
        break;
    case SONG_SIMILAR_SONG:
        free(song_notif->u.similar_songs);
        break;
    case SONG_SIMILAR_ARTIST:
        free(song_notif->u.similar_artist);
        break;
    case SONG_COVERART:
        free(song_notif->u.img.format);
        free(song_notif->u.img.data);
        break;
    case SONG_LYRICS:
        free(song_notif->u.lyrics);
        break;
    case SONG_DATA_TYPE_COUNT:
        break;
    }
}

