#ifndef TEST_PLAYER_FAKE_H
#define TEST_PLAYER_FAKE_H

#include "common.h"

#include "player.h"

struct player_fake {
    struct player player;

    unsigned char got_ctrl[10];
    struct player_ctrl_msg notif;
};

extern struct player_fake player_fake;

#endif
