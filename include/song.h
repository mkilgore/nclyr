#ifndef INCLUDE_SONG_H
#define INCLUDE_SONG_H

#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

enum song_data_type {
    SONG_ARTIST_BIO,
    SONG_SIMILAR_SONG,
    SONG_SIMILAR_ARTIST,
    SONG_COVERART,
    SONG_LYRICS,
    SONG_DATA_TYPE_COUNT
};

struct song_info {
    char *artist;
    char *title;
    char *album;
};

struct song_thread_notify {
    const struct song_info *song;
    enum song_data_type type;
    union {
        char *lyrics;
        char *bio;
        char *similar_songs;
        char *similar_artist;
        struct {
            char *format;
            char *data;
            size_t size;
        } img;
    } u;
};

STATIC_ASSERT(sizeof(struct song_thread_notify) <= PIPE_BUF);

void song_init(struct song_info *sng);
void song_clear(struct song_info *sng);
void song_copy(struct song_info *dest, struct song_info *src);
int song_equal(struct song_info *src1, struct song_info *src2);

void song_thread_start(int notify_fd);
void song_thread_stop(void);

void song_thread_notify_clear(struct song_thread_notify *);

/* list should be terminated with '-1' */
void song_thread_song_lookup(const struct song_info *song, const enum song_data_type *list);

#endif
