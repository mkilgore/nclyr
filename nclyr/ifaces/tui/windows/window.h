#ifndef NCLYR_CURSES_WINDOW_H
#define NCLYR_CURSES_WINDOW_H

#include <ncurses.h>

/* all this stuff has to get put *Before* we include those other files, since
 * they might need it. */

struct nclyr_win;

enum nclyr_mouse_event_type {
    LEFT_PRESSED,
    LEFT_RELEASED,
    LEFT_CLICKED,
    RIGHT_PRESSED,
    RIGHT_RELEASED,
    RIGHT_CLICKED,
    SCROLL_UP,
    SCROLL_DOWN,
    SCROLL_PRESSED,
    SCROLL_RELEASED,
    SCROLL_CLICKED,
};

struct nclyr_mouse_event {
    /* When provided to callback, (x, y) will be relative to the nclyr_win's WINDOW * */
    int x, y;
    enum nclyr_mouse_event_type type;
};

struct nclyr_keypress {
    int ch;
    enum nclyr_mouse_event_type mtype;
    void (*callback) (struct nclyr_win *, int ch, struct nclyr_mouse_event *);
    const char *help_text;
};

#define N_KEYPRESS(chr, call, text) \
    { \
        .ch = (chr), \
        .callback = (call), \
        .help_text = (text) \
    }

#define N_MOUSE(mtyp, call, text) \
    { \
        .ch = KEY_MOUSE, \
        .mtype = (mtyp), \
        .callback = (call), \
        .help_text = (text) \
    }

#define N_END() { .ch = '\0', .mtype = 0, .callback = NULL, .help_text = NULL }


#include "song.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui_internal.h"

struct nclyr_win {
    /* Name to identify this window */
    char *win_name;

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
    unsigned int show_window :1;

    /* Set when the song data required has already been submitted to the query
     * thread */
    unsigned int already_lookedup :1;

    /* If true, the cursor will be shown in the same place as on win */
    unsigned int show_cursor :1;

    /* If true, then 'update' will be called on the window (When nessisary) */
    unsigned int updated :1;

    const enum lyr_data_type *lyr_types; /* This list should be terminated with -1 */
    const struct nclyr_keypress *keypresses; /* This list should be terminated with ch='\0' */

    /* Called when nclyr starts - y and x are window cords, rows and cols are
     * window dimensions */
    void (*init) (struct nclyr_win *);

    /* Called when nclyr closes - any memory or window's held by this nclyr_win
     * should be freed */
    void (*clean) (struct nclyr_win *);

    /* Called when this window is switched too, to allow for an update */
    void (*switch_to) (struct nclyr_win *);

    /* Called right before nclyr sleeps for input */
    void (*update) (struct nclyr_win *);

    /* Called if a terminal resize happened, and this window now has new dimensions */
    void (*resize) (struct nclyr_win *);

    /* Called if the song changed and the old song's data should be purged */
    void (*clear_song_data) (struct nclyr_win *);

    /* Called when we recieve a response from the query thread that this window asked for */
    void (*new_song_data) (struct nclyr_win *, const struct lyr_thread_notify *);

    /* Text command for this window */
    void (*cmd_exec) (struct nclyr_win *, int argc, char **argv);

    void (*new_player_notif) (struct nclyr_win *, enum player_notif_type, struct player_state_full *);

    /* Called when lyr_thread looked has started */
    void (*lookup_started) (struct nclyr_win *);
};


#endif
