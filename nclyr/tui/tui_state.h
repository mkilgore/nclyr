#ifndef NCLYR_TUI_TUI_STATE_H
#define NCLYR_TUI_TUI_STATE_H

#include "song.h"
#include "window.h"
#include "statusline.h"

struct tui_state {
    struct song_info cur_song;

    struct nclyr_win **windows;
    size_t window_count;

    const struct nclyr_keypress *global_keys;

    int show_status :1;
    struct statusline *status;

    int sel_window;
};

void tui_state_change_song (struct tui_state *, struct song_info *);
void tui_state_change_window (struct tui_state *, int win);

#endif
