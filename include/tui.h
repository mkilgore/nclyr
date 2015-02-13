#ifndef INCLUDE_TUI_H
#define INCLUDE_TUI_H

#include "config.h"

struct tui_iface;

extern struct tui_iface tui_iface;

enum tui_config {
    TUI_CONFIG_PLAYLIST,
    TUI_CONFIG_STATUSLINE,
    TUI_CONFIG_TOTAL
};

extern struct config_item tui_conf[];

#endif
