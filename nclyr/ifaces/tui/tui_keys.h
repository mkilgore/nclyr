#ifndef NCLYR_IFACES_TUI_TUI_KEYS_H
#define NCLYR_IFACES_TUI_TUI_KEYS_H

#include "windows/window.h"

void tui_keys_global(struct nclyr_win *, int ch, struct nclyr_mouse_event *);
void tui_keys_player(struct nclyr_win *, int ch, struct nclyr_mouse_event *);

#endif
