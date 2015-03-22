
#include "common.h"

#include "player.h"
#include "tui_internal.h"
#include "cmd_exec.h"

#include "stringcasecmp.h"
#include "a_sprintf.h"
#include "cmd_handle.h"
#include "debug.h"

static void tui_cmd_player_pause(struct tui_iface *tui, int argc, char **argv)
{
    DEBUG_PRINTF("Player: %s\n", player_current()->name);
    player_pause(player_current(), 1);
}

static void tui_cmd_player_play(struct tui_iface *tui, int argc, char **argv)
{
    DEBUG_PRINTF("Player: %s\n", player_current()->name);
    player_play(player_current());
}

static void tui_cmd_player_toggle_pause(struct tui_iface *tui, int argc, char **argv)
{
    DEBUG_PRINTF("Player: %s\n", player_current()->name);
    if (argc == 1 && stringcasecmp(argv[0], "pause") == 0)
        player_toggle_pause(player_current());
}

static void tui_cmd_player_next(struct tui_iface *tui, int argc, char **argv)
{
    player_next(player_current());
}

static void tui_cmd_player_prev(struct tui_iface *tui, int argc, char **argv)
{
    player_prev(player_current());
}

static void tui_cmd_player_seek(struct tui_iface *tui, int argc, char **argv)
{
    size_t seconds;
    if (argc != 1) {
        a_sprintf(&tui->display, "player seek: Please provide seek-time argument");
        return ;
    }

    seconds = strtoll(argv[0], NULL, 0);

    player_seek(player_current(), seconds);
}

static void tui_cmd_player_shuffle(struct tui_iface *tui, int argc, char **argv)
{
    player_shuffle(player_current());
}

static void tui_cmd_player_volume(struct tui_iface *tui, int argc, char **argv)
{
    int volume;
    if (argc != 1) {
        a_sprintf(&tui->display, "player volume: Please provide volume argument");
        return ;
    }

    volume = strtoll(argv[0], NULL, 0);

    if (argv[0][0] == '-' || argv[0][0] == '+')
        player_change_volume(player_current(), volume);
    else
        player_set_volume(player_current(), volume);
}

static void tui_cmd_player_song(struct tui_iface *tui, int argc, char **argv)
{
    int song_no;
    if (argc != 1) {
        a_sprintf(&tui->display, "player song: Please provide song playlist number argument");
        return ;
    }

    song_no = strtoll(argv[0], NULL, 0);

    player_change_song(player_current(), song_no);
}

static struct tui_cmd player_cmds[] = {
    TUI_CMD("pause",   tui_cmd_player_pause),
    TUI_CMD("play",    tui_cmd_player_play),
    TUI_CMD("toggle",  tui_cmd_player_toggle_pause),
    TUI_CMD("next",    tui_cmd_player_next),
    TUI_CMD("prev",    tui_cmd_player_prev),
    TUI_CMD("seek",    tui_cmd_player_seek),
    TUI_CMD("shuffle", tui_cmd_player_shuffle),
    TUI_CMD("volume",  tui_cmd_player_volume),
    TUI_CMD("song",    tui_cmd_player_song),
    TUI_CMD_END()
};

void tui_cmd_handle_player(struct tui_iface *tui, int argc, char **argv)
{
    DEBUG_PRINTF("Player cmd: %s\n", argv[0]);
    tui_cmd_exec_args(tui, player_cmds, argc, argv);
}

