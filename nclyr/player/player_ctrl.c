
#include "common.h"

#include <string.h>
#include <unistd.h>

#include "player.h"

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
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_SEEK, .u.seek_pos = pos };
    p->ctrls.ctrl(p, &msg);
}

void player_shuffle (struct player *p)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_SHUFFLE };
    p->ctrls.ctrl(p, &msg);
}

void player_set_volume (struct player *p, size_t volume)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_SET_VOLUME, .u.volume = volume };
    p->ctrls.ctrl(p, &msg);
}

void player_change_volume (struct player *p, int change)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_CHANGE_VOLUME, .u.vol_change = change };
    p->ctrls.ctrl(p, &msg);
}

void player_change_song (struct player *p, int song_pos)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_CHANGE_SONG, .u.song_pos = song_pos };
    p->ctrls.ctrl(p, &msg);
}

void player_remove_song (struct player *p, int song_pos)
{
    struct player_ctrl_msg msg = { .type = PLAYER_CTRL_REMOVE_SONG, .u.song_pos = song_pos };
    p->ctrls.ctrl(p, &msg);
}

