#ifndef NCLYR_TUI_TUI_STATE_H
#define NCLYR_TUI_TUI_STATE_H

#include "song.h"
#include "player.h"
#include "tui/window.h"
#include "tui/statusline.h"

struct tui_state {
};

void tui_state_change_window (struct tui_state *, struct nclyr_win *);

#endif
