
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "song.h"
#include "playlist.h"
#include "player.h"
#include "debug.h"

void player_notification_clear(struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_SONG:
        song_clear(&notif->u.song);
        break;

    case PLAYER_PLAYLIST:
        playlist_clear(&notif->u.playlist);
        break;

    case PLAYER_STATE:
    case PLAYER_NO_SONG:
    case PLAYER_IS_DOWN:
    case PLAYER_IS_UP:
    case PLAYER_SEEK:
    case PLAYER_VOLUME:
        break;
    }
}

