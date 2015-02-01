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

    void (*sfree) (struct song_info *);
    struct song_info *(*copy) (const struct song_info *);
};

void song_init(struct song_info *sng);
void song_info_clear(struct song_info *sng);
int song_equal(const struct song_info *src1, const struct song_info *src2);

#define song_free(sng) \
    do { \
        if ((sng)) \
            (sng)->sfree((sng)); \
    } while (0)

#define song_copy(sng) (((sng))? (sng)->copy((sng)): NULL)

extern struct song_info song_empty;

#endif
