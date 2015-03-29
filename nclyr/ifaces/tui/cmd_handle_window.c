
#include "common.h"

#include "tui_internal.h"
#include "cmd_exec.h"

#include "stringcasecmp.h"
#include "a_sprintf.h"
#include "cmd_handle.h"
#include "debug.h"

static void window_command(struct tui_iface *tui, int argc, char **argv)
{
    if (tui->sel_window->cmd_exec)
        (tui->sel_window->cmd_exec) (tui->sel_window, argc, argv);
    else
        a_sprintf(&tui->display, "Window %s doesn't take any command", tui->sel_window->win_name);
}

static void window_add(struct tui_iface *tui, int argc, char **argv)
{
    struct nclyr_win *w;
    int window_n;

    if (argc != 1) {
        a_sprintf(&tui->display, "window add: Please supply the window number to add");
        return ;
    }

    window_n = strtoll(argv[0], NULL, 0);
    w = tui_window_new(tui, window_descs + window_n);
    tui_window_add(tui, w);
    tui_change_window(tui, w);
}

static void window_delete(struct tui_iface *tui, int argc, char **argv)
{
    struct nclyr_win *w;
    int window_n;

    if (argc != 1) {
        a_sprintf(&tui->display, "window delete: Please supply the window number to delete");
        return ;
    }

    window_n = strtoll(argv[0], NULL, 0);
    w = tui->windows[window_n];
    tui_window_del(tui, window_n);

    w->clean(w);
    free(w);
}

static void window_sel(struct tui_iface *tui, int argc, char **argv)
{
    tui_change_window(tui, tui->manager_win);
}

static struct tui_cmd window_cmds[] = {
    TUI_CMD("command",  window_command),
    TUI_CMD("add",      window_add),
    TUI_CMD("delete",   window_delete),
    TUI_CMD("select",   window_sel),
    TUI_CMD_END()
};

void tui_cmd_handle_window(struct tui_iface *tui, int argc, char **argv)
{
    if (argc)
        tui_cmd_exec_args(tui, window_cmds, argc, argv);
    else
        window_sel(tui, argc, argv);
}

