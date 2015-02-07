
#include "common.h"

#include "player.h"
#include "test/player_fake.h"
#include "debug.h"

static void fake_player_thread_start(struct player *p)
{
    return ;
}

static void fake_player_thread_stop(struct player *p)
{
    return ;
}

static void fake_player_ctrl(struct player *p, const struct player_ctrl_msg *msg)
{
    struct player_fake *player = container_of(p, struct player_fake, player);
    player->got_ctrl[msg->type] = 1;
    player->notif = *msg;
}

struct player_fake player_fake = {
    .player = {
        .name = "fake player",
        .start_thread = fake_player_thread_start,
        .stop_thread = fake_player_thread_stop,
        .ctrls = {
            .ctrl = fake_player_ctrl,
            .has_pause = 1,
            .has_toggle_pause = 1,
            .has_play = 1,
            .has_next = 1,
            .has_prev = 1,
            .has_shuffle = 1,
            .has_set_volume = 1,
            .has_change_volume = 1,
        }
    },
};

