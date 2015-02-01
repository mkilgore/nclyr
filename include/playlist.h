#ifndef INCLUDE_PLAYLIST_H
#define INCLUDE_PLAYLIST_H

#include <string.h>
#include <stdlib.h>

#include "song.h"

struct playlist {
    struct song_info **songs;
    size_t song_count;
};

void playlist_init(struct playlist *);
void playlist_copy(struct playlist *dest, struct playlist *src);
void playlist_clear(struct playlist *);

static inline void playlist_move(struct playlist *dest, struct playlist *src)
{
    memcpy(dest, src, sizeof(*dest));
    memset(src, 0, sizeof(*src));
}

#endif
