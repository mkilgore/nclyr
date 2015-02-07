
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "song.h"
#include "playlist.h"
#include "player.h"

void player_state_full_clear(struct player_state_full *state)
{
    song_free(state->song);
    playlist_clear(&state->playlist);
}

void player_state_full_update(struct player_state_full *state, struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_SONG:
        song_free(state->song);
        state->song = notif->u.song;
        notif->u.song = NULL;
        break;

    case PLAYER_PLAYLIST:
        playlist_clear(&state->playlist);
        playlist_move(&state->playlist, &notif->u.playlist);
        break;

    case PLAYER_STATE:
        state->state = notif->u.state;
        break;

    case PLAYER_NO_SONG:
        song_free(state->song);
        state->song = NULL;
        break;

    case PLAYER_IS_DOWN:
        state->is_up = 0;
        break;

    case PLAYER_IS_UP:
        state->is_up = 1;
        break;

    case PLAYER_SEEK:
        state->seek_pos = notif->u.seek_pos;
        break;

    case PLAYER_VOLUME:
        state->volume = notif->u.volume;
        break;

    case PLAYER_SONG_POS:
        state->song_pos = notif->u.song_pos;
        break;
    }
}


