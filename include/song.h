#ifndef INCLUDE_SONG_H
#define INCLUDE_SONG_H

#include "common.h"

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

#endif
