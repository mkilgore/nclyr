#ifndef NCLYR_PLAYERS_MPD_MPD_SONG_H
#define NCLYR_PLAYERS_MPD_MPD_SONG_H

#include "song.h"

struct song_info_mpd {
    struct song_info song;
};

void song_info_mpd_init(struct song_info_mpd *);
void song_info_mpd_clear(struct song_info_mpd *);

#endif
