
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "song.h"
#include "playlist.h"
#include "player.h"
#include "debug.h"

void player_send_is_up(struct player *player)
{
    struct player_notification notif = { .type = PLAYER_IS_UP };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_is_down(struct player *player)
{
    struct player_notification notif = { .type = PLAYER_IS_DOWN };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_state(struct player *player, enum player_state state)
{
    struct player_notification notif = { .type = PLAYER_STATE, .u.state = state };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_no_song(struct player *player)
{
    struct player_notification notif = { .type = PLAYER_NO_SONG };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_cur_song(struct player *player, struct song_info *song)
{
    struct player_notification notif = { .type = PLAYER_SONG, .u.song = song };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_seek(struct player *player, size_t seek)
{
    struct player_notification notif = { .type = PLAYER_SEEK, .u.seek_pos = seek };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_volume(struct player *player, size_t volume)
{
    struct player_notification notif = { .type = PLAYER_VOLUME, .u.volume = volume };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_playlist(struct player *player, struct playlist *playlist)
{
    struct player_notification notif = { .type = PLAYER_PLAYLIST, .u.playlist = *playlist };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_song_pos(struct player *player, int song_pos)
{
    struct player_notification notif = { .type = PLAYER_SONG_POS, .u.song_pos = song_pos };
    write(player->notify_fd, &notif, sizeof(notif));
}

void player_send_directory(struct player *player, struct directory *dir)
{
    struct player_notification notif = { .type = PLAYER_DIRECTORY, .u.dir = *dir };
    write(player->notify_fd, &notif, sizeof(notif));
}

