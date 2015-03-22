#ifndef NCLYR_IFACES_TUI_CMD_HANDLE_H
#define NCLYR_IFACES_TUI_CMD_HANDLE_H

#include "tui.h"
#include "tui_internal.h"

void tui_cmd_handle_player(struct tui_iface *tui, int argc, char **argv);
void tui_cmd_handle_window(struct tui_iface *tui, int argc, char **argv);
void tui_cmd_handle_tui(struct tui_iface *tui, int argc, char **argv);

#endif
