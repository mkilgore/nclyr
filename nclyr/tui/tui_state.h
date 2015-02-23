#ifndef NCLYR_TUI_TUI_STATE_H
#define NCLYR_TUI_TUI_STATE_H

#include "song.h"
#include "player.h"
#include "windows/window.h"
#include "tui_internal.h"

void tui_state_change_window (struct tui_iface *, struct nclyr_win *);
chtype tui_get_chtype_from_window(WINDOW *win);

#endif
