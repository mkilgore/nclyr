#ifndef NCLYR_LYR_THREAD_H
#define NCLYR_LYR_THREAD_H

#include "common.h"

#include <limits.h>

#include "song.h"

enum lyr_data_type {
    LYR_ARTIST_BIO,
    LYR_SIMILAR_SONG,
    LYR_SIMILAR_ARTIST,
    LYR_COVERART,
    LYR_LYRICS,
    LYR_DATA_TYPE_COUNT
};

struct lyr_thread_notify {
    struct song_info song;
    enum lyr_data_type type;
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
STATIC_ASSERT(sizeof(struct lyr_thread_notify) <= PIPE_BUF);

#ifdef CONFIG_LIB_GLYR
void lyr_thread_start(int notify_fd);
void lyr_thread_stop(void);

void lyr_thread_notify_clear(struct lyr_thread_notify *);

/* list should be terminated with '-1' */
void lyr_thread_song_lookup(const struct song_info *song, const enum lyr_data_type *list);
#else
static inline void lyr_thread_start(int notify_fd) { return ; }
static inline void lyr_thread_stop(void) { return ; }

static inline void lyr_thread_notify_clear(struct lyr_thread_notify *notify) { return ; }
static inline void lyr_thread_song_lookup(const struct song_info *song, const enum lyr_data_type *list) { return ; }
#endif

#endif
