
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "player.h"
#include "pianobar/pianobar.h"
#include "mpd/mpd.h"

struct player *players[] = {
#if CONFIG_PLAYER_PIANOBAR
    &pianobar_player.player,
#endif
#if CONFIG_PLAYER_MPD
    &mpd_player.player,
#endif
    NULL
};

static struct player *sel_player = NULL;

struct player *player_find(const char *name)
{
    struct player **player;
    for (player = players; *player; player++)
        if (strcmp((*player)->name, name) == 0)
            return *player;

    return NULL;
}

void player_set_current(struct player *player)
{
    sel_player = player;
}

struct player *player_current(void)
{
    return sel_player;
}

void player_start_thread(struct player *player, int pipefd)
{
    if (!player)
        return ;

    player->notify_fd = pipefd;
    player->start_thread(player);
    return ;
}

void player_stop_thread(struct player *player)
{
    if (!player)
        return ;

    player->stop_thread(player);
    return ;
}

void player_notification_free(struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_SONG:
        song_clear(&notif->u.song);
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

void player_pause (struct player *p, int pause)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_PAUSE, .u.pause = pause };
    p->ctrls.ctrl(p, &msg);
}

void player_toggle_pause (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_TOGGLE_PAUSE };
    p->ctrls.ctrl(p, &msg);
}

void player_play (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_PLAY };
    p->ctrls.ctrl(p, &msg);
}

void player_next (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_NEXT };
    p->ctrls.ctrl(p, &msg);
}

void player_prev (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_PREV };
    p->ctrls.ctrl(p, &msg);
}

void player_seek (struct player *p, size_t pos)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_PREV, .u.seek_pos = pos };
    p->ctrls.ctrl(p, &msg);
}

void player_shuffle (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_SHUFFLE };
    p->ctrls.ctrl(p, &msg);
}

void player_set_volume (struct player *p, size_t volume)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_PREV, .u.volume = volume };
    p->ctrls.ctrl(p, &msg);
}

void player_change_volume (struct player *p, int change)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_CHANGE_VOLUME, .u.vol_change = change };
    p->ctrls.ctrl(p, &msg);
}


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

void player_send_cur_song(struct player *player, const struct song_info *song)
{
    struct player_notification notif = { .type = PLAYER_SONG };
    song_copy(&notif.u.song, song);
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

