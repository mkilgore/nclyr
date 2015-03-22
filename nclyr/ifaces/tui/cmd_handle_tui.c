
#include "common.h"

#include "tui_internal.h"
#include "cmd_exec.h"

#include "stringcasecmp.h"
#include "a_sprintf.h"
#include "cmd_handle.h"
#include "debug.h"

static void tui_next(struct tui_iface *tui, int argc, char **argv)
{
    tui->sel_window_index = (tui->sel_window_index + 1) % tui->window_count;
    tui_change_window(tui, tui->windows[tui->sel_window_index]);
}

static void tui_prev(struct tui_iface *tui, int argc, char **argv)
{
    tui->sel_window_index = tui->sel_window_index - 1;
    if (tui->sel_window_index == -1)
        tui->sel_window_index = tui->window_count - 1;

    tui_change_window(tui, tui->windows[tui->sel_window_index]);
}

static struct tui_cmd tui_cmds[] = {
    TUI_CMD("next", tui_next),
    TUI_CMD("prev", tui_prev),
    TUI_CMD_END()
};

void tui_cmd_handle_tui(struct tui_iface *tui, int argc, char **argv)
{
    tui_cmd_exec_args(tui, tui_cmds, argc, argv);
}

