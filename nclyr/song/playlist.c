
#include "common.h"

#include <stdlib.h>

#include "song.h"
#include "playlist.h"
#include "debug.h"

void playlist_init(struct playlist *playlist)
{
    memset(playlist, 0, sizeof(*playlist));
}

void playlist_clear(struct playlist *playlist)
{
    int i;
    for (i = 0; i < playlist->song_count; i++)
        song_free(playlist->songs[i]);
    free(playlist->songs);
}

void playlist_copy(struct playlist *dest, struct playlist *src)
{
    int i;
    dest->song_count = src->song_count;
    dest->songs = malloc(sizeof(*dest->songs) * dest->song_count);
    for (i = 0; i < dest->song_count; i++)
        dest->songs[i] = song_copy(src->songs[i]);
}

