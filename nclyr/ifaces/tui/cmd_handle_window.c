
#include "common.h"

#include "tui_internal.h"
#include "cmd_exec.h"

#include "stringcasecmp.h"
#include "a_sprintf.h"
#include "cmd_handle.h"
#include "debug.h"

void tui_cmd_handle_window(struct tui_iface *tui, int argc, char **argv)
{
    if (tui->sel_window->cmd_exec)
        (tui->sel_window->cmd_exec) (tui->sel_window, argc, argv);
    else
        a_sprintf(&tui->display, "Window %s doesn't take any command", tui->sel_window->win_name);
}

