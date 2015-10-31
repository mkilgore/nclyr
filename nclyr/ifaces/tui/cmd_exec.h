#ifndef NCLYR_IFACES_TUI_CMD_EXEC_H
#define NCLYR_IFACES_TUI_CMD_EXEC_H

#include "tui.h"
#include "tui_internal.h"

struct tui_cmd {
    const char *cmd;

    void (*callback) (struct tui_iface *tui, int argc, char **argv);
    const char *description;
};

#define TUI_CMD(command, call) { .cmd = (command), .callback = (call) }
#define TUI_CMD_END() { .cmd = NULL, .callback = NULL }

void tui_cmd_exec(struct tui_iface *tui, const struct tui_cmd *cmds, char *cmd);
void tui_cmd_exec_args(struct tui_iface *tui, const struct tui_cmd *cmds, int argc, char **argv);

#endif
