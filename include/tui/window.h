#ifndef NCLYR_CURSES_WINDOW_H
#define NCLYR_CURSES_WINDOW_H

#include <ncurses.h>

struct nclyr_keypress;
struct nclyr_win;

#include "song.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui.h"

struct nclyr_win {
    /* Name to identify this window */
    const char *win_name;

    /* We store a pointer to the tui_iface in every window so it can access it */
    struct tui_iface *tui;

    /* If there is no input provided to wake-up nclyr from sleeping, then nclyr
     * will sleep for at most 'timeout' milliseconds. Use -1 if nclyr should
     * sleep forever until something happens. */
    int timeout;

    /* The actual ncurses WINDOW * associated with this window */
    WINDOW *win;

    /* Set when this window should be shown and allowed to be switched too.  If
     * this window should be displayed for whatever reason, this flag should be
     * set */
    int show_window :1;

    /* Set when the song data required has already been submitted to the query
     * thread */
    int already_lookedup :1;

    /* If true, the cursor will be shown in the same place as on win */
    int show_cursor :1;

    const enum lyr_data_type *lyr_types; /* This list should be terminated with -1 */
    const struct nclyr_keypress *keypresses; /* This list should be terminated with ch='\0' */

    /* Called when nclyr starts - y and x are window cords, rows and cols are
     * window dimensions */
    void (*init) (struct nclyr_win *, int y, int x, int rows, int cols);

    /* Called when nclyr closes - any memory or window's held by this nclyr_win
     * should be freed */
    void (*clean) (struct nclyr_win *);

    /* Called when this window is switched too, to allow for an update */
    void (*switch_to) (struct nclyr_win *);

    /* Called right before nclyr sleeps for input */
    void (*update) (struct nclyr_win *);

    /* Called if a terminal resize happened, and this window now has new dimensions */
    void (*resize) (struct nclyr_win *, int y, int x, int rows, int cols);

    /* Called if the song changed and the old song's data should be purged */
    void (*clear_song_data) (struct nclyr_win *);

    /* Called when we recieve a response from the query thread that this window asked for */
    void (*new_song_data) (struct nclyr_win *, const struct lyr_thread_notify *);

    void (*new_player_notif) (struct nclyr_win *, enum player_notif_type, struct player_state_full *);
};

struct nclyr_keypress {
    int ch;
    void (*callback) (struct nclyr_win *, int ch);
    const char *help_text;
};

#endif
