
#include "common.h"

#include "mpd.h"
#include "song_info_mpd.h"
#include "debug.h"

void song_info_mpd_clear(struct song_info_mpd *song)
{
    song_info_clear(&song->song);
}

static void song_info_mpd_free(struct song_info *song)
{
    struct song_info_mpd *msong = container_of(song, struct song_info_mpd, song);
    song_info_mpd_clear(msong);
    free(song);
}

static struct song_info *song_info_mpd_copy(const struct song_info *song)
{
    struct song_info_mpd *new_msong = malloc(sizeof(*new_msong));
    song_info_mpd_init(new_msong);

    song_info_copy(&new_msong->song, song);

    return &new_msong->song;
}

void song_info_mpd_init(struct song_info_mpd *song)
{
    memset(song, 0, sizeof(*song));
    song_init(&song->song);

    song->song.sfree = song_info_mpd_free;
    song->song.copy = song_info_mpd_copy;
}

