
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "player.h"
#include "iface.h"
#include "test/player_fake.h"
#include "test/test.h"

int test_ctrls(void)
{
    int ret = 0;
    struct player *p = &player_fake.player;
    struct nclyr_pipes pipes;

    nclyr_pipes_open(&pipes);

    player_pause(p, 20);
    ret += test_assert_with_name("pause", player_fake.got_ctrl[PLAYER_CTRL_PAUSE] == 1);
    ret += test_assert_with_name("pause", player_fake.notif.u.pause == 20);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_toggle_pause(p);
    ret += test_assert_with_name("toggle pause", player_fake.got_ctrl[PLAYER_CTRL_TOGGLE_PAUSE] == 1);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_play(p);
    ret += test_assert_with_name("play", player_fake.got_ctrl[PLAYER_CTRL_PLAY] == 1);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_next(p);
    ret += test_assert_with_name("next", player_fake.got_ctrl[PLAYER_CTRL_NEXT] == 1);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_prev(p);
    ret += test_assert_with_name("prev", player_fake.got_ctrl[PLAYER_CTRL_PREV] == 1);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_seek(p, 120);
    ret += test_assert_with_name("seek", player_fake.got_ctrl[PLAYER_CTRL_SEEK] == 1);
    ret += test_assert_with_name("seek", player_fake.notif.u.seek_pos == 120);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_shuffle(p);
    ret += test_assert_with_name("shuffle", player_fake.got_ctrl[PLAYER_CTRL_SHUFFLE] == 1);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_set_volume(p, 120);
    ret += test_assert_with_name("set volume", player_fake.got_ctrl[PLAYER_CTRL_SET_VOLUME] == 1);
    ret += test_assert_with_name("set volume", player_fake.notif.u.volume == 120);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_change_volume(p, 80);
    ret += test_assert_with_name("change volume", player_fake.got_ctrl[PLAYER_CTRL_CHANGE_VOLUME] == 1);
    ret += test_assert_with_name("change volume", player_fake.notif.u.vol_change == 80);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    player_change_song(p, 2);
    ret += test_assert_with_name("change song", player_fake.got_ctrl[PLAYER_CTRL_CHANGE_SONG] == 1);
    ret += test_assert_with_name("change song", player_fake.notif.u.song_pos == 2);
    memset(player_fake.got_ctrl, 0, sizeof(player_fake.got_ctrl));

    nclyr_pipes_close(&pipes);

    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
        { test_ctrls, "Player ctrl test" },
    };

    ret = run_tests("Test Player", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}

