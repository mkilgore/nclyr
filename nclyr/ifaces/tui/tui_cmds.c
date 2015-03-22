
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "tui.h"
#include "tui_internal.h"
#include "cmd_exec.h"
#include "cmd_handle.h"

#include "tui_cmds.h"
#include "debug.h"

void tui_cmd_quit(struct tui_iface *tui, int argc, char **argv)
{
    tui->exit_flag = 1;
}

