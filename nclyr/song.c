
#include "common.h"

#include <string.h>

#include "nstrdup.h"
#include "song.h"

void song_info_clear(struct song_info *sng)
{
    if (!sng)
        return ;
    free(sng->title);
    free(sng->artist);
    free(sng->album);
}

static void song_info_free(struct song_info *sng)
{
    song_info_clear(sng);
    free(sng);
}

static struct song_info *song_info_copy(const struct song_info *src)
{
    struct song_info *dest = malloc(sizeof(*src));

    song_init(dest);

    dest->title = nstrdup(src->title);
    dest->artist = nstrdup(src->artist);
    dest->album = nstrdup(src->album);

    dest->duration = src->duration;

    return dest;
}

void song_init(struct song_info *sng)
{
    memset(sng, 0, sizeof(*sng));
    sng->sfree = song_info_free;
    sng->copy = song_info_copy;
}

int song_equal(const struct song_info *src1, const struct song_info *src2)
{
    if (!src1 || !src2)
        return 0;
    if (!src1->title || !src2->title || !!strcmp(src1->title, src2->title))
        return 0;
    if (!src1->artist || !src2->artist || !!strcmp(src1->artist, src2->artist))
        return 0;
    if (!src1->album || !src2->album || !!strcmp(src1->album, src2->album))
        return 0;
    if (src1->duration != src2->duration)
        return 0;
    return 1;
}

