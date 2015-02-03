
#include "common.h"

#include <string.h>

#include "nstrdup.h"
#include "song.h"

void song_info_clear(struct song_info *sng)
{
    if (!sng)
        return ;
    free(sng->name);
    free(sng->tag.title);
    free(sng->tag.artist);
    free(sng->tag.album);
}

static void song_info_free(struct song_info *sng)
{
    song_info_clear(sng);
    free(sng);
}

void song_info_copy(struct song_info *dest, const struct song_info *src)
{
    dest->name = nstrdup(src->name);
    dest->tag.title = nstrdup(src->tag.title);
    dest->tag.artist = nstrdup(src->tag.artist);
    dest->tag.album = nstrdup(src->tag.album);

    dest->duration = src->duration;
}

static struct song_info *song_info_alloc_copy(const struct song_info *src)
{
    struct song_info *dest = malloc(sizeof(*src));
    song_init(dest);

    song_info_copy(dest, src);

    return dest;
}

void song_init(struct song_info *sng)
{
    memset(sng, 0, sizeof(*sng));
    sng->sfree = song_info_free;
    sng->copy = song_info_alloc_copy;
}

int song_equal(const struct song_info *src1, const struct song_info *src2)
{
    if (!src1 || !src2)
        return 0;
    if (!src1->name || !src2->name || !!strcmp(src1->name, src2->name))
        return 0;
    if (!src1->tag.title || !src2->tag.title || !!strcmp(src1->tag.title, src2->tag.title))
        return 0;
    if (!src1->tag.artist || !src2->tag.artist || !!strcmp(src1->tag.artist, src2->tag.artist))
        return 0;
    if (!src1->tag.album || !src2->tag.album || !!strcmp(src1->tag.album, src2->tag.album))
        return 0;
    if (src1->duration != src2->duration)
        return 0;
    return 1;
}

