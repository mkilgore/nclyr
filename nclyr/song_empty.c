
#include "common.h"

#include <string.h>

#include "song.h"

struct song_info *song_empty_copy(const struct song_info *sng)
{
    return &song_empty;
}

void song_empty_free(struct song_info *sng)
{
    return ;
}

/* The 'empty song' is a completely empty song structure, but which can still
 * act in place of a song */
struct song_info song_empty = {
    .title = "",
    .artist = "",
    .album = "",
    .duration = 0,
    .copy = song_empty_copy,
    .sfree = song_empty_free,
};



