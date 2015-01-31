#ifndef INCLUDE_SONG_H
#define INCLUDE_SONG_H

#include "common.h"

#include <string.h>
#include <stdlib.h>

struct song_info {
    char *artist;
    char *title;
    char *album;
    size_t duration;
};

void song_init(struct song_info *sng);
void song_clear(struct song_info *sng);
void song_copy(struct song_info *dest, const struct song_info *src);
int song_equal(const struct song_info *src1, const struct song_info *src2);

/* "Moves" the data from being owned by one song_info to another
 * Useful when you want to copy from one song_info to another, but don't want
 * to bother allocating one song_info just to immedieatly freeing the other */
static inline void song_move(struct song_info *dest, struct song_info *src)
{
    memcpy(dest, src, sizeof(*dest));
    memset(src, 0, sizeof(*src));
}

#endif
