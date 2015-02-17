
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui_internal.h"
#include "tui_color.h"
#include "windows/window.h"
#include "windows/statusline.h"
#include "windows/clock_win.h"
#include "windows/lyrics_win.h"
#include "windows/help_win.h"
#include "windows/artist_win.h"
#include "windows/playlist_win.h"
#include "windows/config_win.h"

#include "tui.h"
#include "debug.h"

static struct nclyr_win *nclyr_windows[] = {
    &playlist_window.super_win,
#if CONFIG_LIB_GLYR
    &lyrics_window.super_win,
    &artist_window.super_win,
#endif
    &config_window.super_win,
    &help_window.super_win,
    &clock_window.super_win,
    NULL
};

static void player_keys(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct player *player = player_current();

    DEBUG_PRINTF("Got player key: %c\n", ch);

    switch (ch) {
    case ' ':
        player_toggle_pause(player);
        break;

    case 'p':
        player_prev(player);
        break;

    case 'n':
        player_next(player);
        break;

    case '+':
        player_change_volume(player, 1);
        break;

    case '-':
        player_change_volume(player, -1);
        break;
    }
}

static void global_keys(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct tui_iface *tui = win->tui;

    DEBUG_PRINTF("Got global key: %c\n", ch);
    if (ch == 'w') {
        tui->sel_window_index = (tui->sel_window_index + 1) % tui->window_count;
        tui_change_window(tui, tui->windows[tui->sel_window_index]);
    } else if (ch == 'q') {
        tui->sel_window_index = tui->sel_window_index - 1;
        if (tui->sel_window_index == -1)
            tui->sel_window_index = tui->window_count - 1;

        tui_change_window(tui, tui->windows[tui->sel_window_index]);
    } else if (ch == 'Q') {
        tui->exit_flag = 1;
    } else if (ch == ':') {
        tui->grab_input = 1;
        memset(tui->inp_buf, 0, tui->inp_buf_len);
    }
}

static void handle_player_fd(struct tui_iface *tui, int playerfd)
{
    struct player_notification notif;
    struct nclyr_win **win;

    read(playerfd, &notif, sizeof(notif));

    DEBUG_PRINTF("Got Player Notification in TUI thread\n");

    player_state_full_update(&tui->state, &notif);

    if (notif.type == PLAYER_SONG) {
        for (win = tui->windows; *win; win++) {
            if ((*win)->clear_song_data)
                (*win)->clear_song_data(*win);
            (*win)->already_lookedup = 0;
        }

        tui->sel_window->already_lookedup = 1;
        if (tui->state.song)
            lyr_thread_song_lookup(tui->state.song, tui->sel_window->lyr_types);
    }

    for (win = tui->windows; *win; win++) {
        if ((*win)->new_player_notif)
            (*win)->new_player_notif(*win, notif.type, &tui->state);
    }

    tui->status->player_notif(tui->status, notif.type, &tui->state);

    player_notification_clear(&notif);
    return ;
}

static void handle_notify_fd(struct tui_iface *tui, int notifyfd)
{
    struct nclyr_win **win;
    struct lyr_thread_notify song_notif;
    const enum lyr_data_type *song_data;

    read(notifyfd, &song_notif, sizeof(song_notif));

    DEBUG_PRINTF("Got Lyr-thread notification: %d\n", song_notif.type);

    if (!song_equal(song_notif.song, tui->state.song)) {
        DEBUG_PRINTF("Song didn't match!\n");
        goto clear_song_notify;
    }

    for (win = tui->windows; *win; win++)
        for (song_data = (*win)->lyr_types; *song_data != LYR_DATA_TYPE_COUNT; song_data++)
            if (*song_data == song_notif.type)
                (*win)->new_song_data(*win, &song_notif);

clear_song_notify:
    lyr_thread_notify_clear(&song_notif);
    return ;
}

static void handle_signal_fd(struct tui_iface *tui, int signalfd)
{
    int sig, rows;
    struct nclyr_win **win;

    rows = getmaxy(tui->status->win);

    read(signalfd, &sig, sizeof(sig));
    switch (sig) {
    case SIGINT:
        tui->exit_flag = 1;
        break;
    case SIGWINCH:
        /* endwin() and refresh() are called to force ncurses to resize
         * stdscr and display the new window. */
        endwin();
        refresh();

        for (win = tui->windows; *win; win++) {
            struct nclyr_win *w = *win;
            delwin(w->win);
            w->win = newwin(LINES - rows - 2, COLS, rows + 1, 0);
            touchwin(w->win);
            w->updated = 1;

            if (w->resize)
                w->resize(w);
        }

        tui->status->resize(tui->status, COLS);
        break;
    }
    return ;
}

static void handle_mouse(struct tui_iface *tui)
{
    struct nclyr_mouse_event mevent;
    struct nclyr_win *win = tui->sel_window;
    const struct nclyr_keypress *key;
    int x, y, v;
    MEVENT me;

    v = getmouse(&me);
    x = me.x;
    y = me.y;

    DEBUG_PRINTF("Mouse event: 0x%08x - %d\n", me.bstate, v);

    if (!wmouse_trafo(win->win, &y, &x, FALSE)) {
        DEBUG_PRINTF("X and Y wern't inside the window\n");
        return ;
    }

    DEBUG_PRINTF("(x, y) = (%d, %d)\n", x, y);

    memset(&mevent, 0, sizeof(mevent));
    mevent.x = x;
    mevent.y = y;

    switch (me.bstate) {
    case BUTTON1_PRESSED:
        mevent.type = LEFT_PRESSED;
        break;
    case BUTTON1_RELEASED:
        if (tui->last_mevent == LEFT_PRESSED)
            mevent.type = LEFT_CLICKED;
        else
            mevent.type = LEFT_RELEASED;
        break;
    case BUTTON3_PRESSED:
        mevent.type = RIGHT_PRESSED;
        break;
    case BUTTON3_RELEASED:
        if (tui->last_mevent == RIGHT_PRESSED)
            mevent.type = RIGHT_CLICKED;
        else
            mevent.type = RIGHT_RELEASED;
        break;
    case BUTTON4_PRESSED:
        mevent.type = SCROLL_UP;
        break;
    case BUTTON2_PRESSED:
        mevent.type = SCROLL_DOWN;
        break;
    case REPORT_MOUSE_POSITION:
        mevent.type = tui->last_mevent;
        break;
    }

    tui->last_mevent = mevent.type;

    for (key = win->keypresses; key->ch !=  '\0'; key++) {
        if (key->ch == KEY_MOUSE) {
            if (key->mtype == mevent.type) {
                key->callback(tui->sel_window, KEY_MOUSE, &mevent);
                break;
            }
        }
    }
}

static void handle_stdin_fd(struct tui_iface *tui, int stdinfd)
{
    int ch = getch();
    const struct nclyr_keypress *key, **cur_keylist;
    const struct nclyr_keypress *keylist[] = {
        tui->global_keys,
        tui->sel_window->keypresses,
        NULL
    };

    if (ch == KEY_MOUSE) {
        handle_mouse(tui);
        goto found_key;
    }

    if (tui->grab_input) {
        if (ch == KEY_BACKSPACE) {
            tui->inp_buf[strlen(tui->inp_buf) - 1] = '\0';
        } else if (ch == KEY_ENTER || ch == '\n') {
            tui->grab_input = 0;
        } else {
            size_t len = strlen(tui->inp_buf);
            tui->inp_buf[len] = ch;
            tui->inp_buf[len + 1] = '\0';
        }
        goto found_key;
    }

    for (cur_keylist = keylist; *cur_keylist != NULL; cur_keylist++) {
        for (key = *cur_keylist; key->ch != '\0'; key++) {
            if (key->ch == ch) {
                key->callback(tui->sel_window, ch, NULL);
                goto found_key;
            }
        }

    }

found_key:
    return ;
}

static void tui_main_loop(struct nclyr_iface *iface, struct nclyr_pipes *pipes)
{
    int i = 0;
    int rows;
    char inp_buf[200];
    struct tui_iface *tui = container_of(iface, struct tui_iface, iface);
    struct pollfd main_notify[4];
    struct nclyr_win **win;

    initscr();
    cbreak();
    keypad(stdscr, 1);
    nodelay(stdscr, TRUE);
    noecho();
    if (has_colors()) {
        DEBUG_PRINTF("Terminal supports colors\n");
        start_color();
        use_default_colors();
    } else {
        DEBUG_PRINTF("!!!Terminal doesn't support colors!!!\n");
    }

    mousemask(ALL_MOUSE_EVENTS, NULL);
    /* A zero internal is nessisary to correctly detect scrolling */
    mouseinterval(0);

    if (COLOR_PAIRS <= 64)
        cons_color_set_default( &(struct cons_color_pair) { CONS_COLOR_WHITE, CONS_COLOR_BLACK });

    tui_color_init();

    tui->inp_buf = inp_buf;
    tui->inp_buf_len = sizeof(inp_buf);

    tui->cfg = tui_config_get_root();

    tui->status->tui = tui;
    tui->status->init(tui->status, COLS);

    rows = getmaxy(tui->status->win);

    for (win = tui->windows; *win; win++) {
        struct nclyr_win *w = *win;
        w->tui = tui;
        w->win = newwin(LINES - rows - 2, COLS, rows + 1, 0);
        touchwin(w->win);
        w->updated = 1;

        if (w->init)
            w->init(w);
    }

    main_notify[0].fd = pipes->player[0];
    main_notify[0].events = POLLIN;

    main_notify[1].fd = pipes->lyr[0];
    main_notify[1].events = POLLIN;

    main_notify[2].fd = pipes->sig[0];
    main_notify[2].events = POLLIN;

    main_notify[3].fd = STDIN_FILENO;
    main_notify[3].events = POLLIN;

    tui->sel_window = tui->windows[tui->sel_window_index];

    while (!tui->exit_flag) {
        curs_set(0);

        for (i = 0; i < COLS; i++)
            mvaddch(rows, i, ACS_HLINE);

        move(rows, COLS / 2 - strlen(tui->sel_window->win_name) / 2 - 2);

        addch(ACS_RTEE);
        printw(" %s ", tui->sel_window->win_name);
        addch(ACS_LTEE);

        if (tui->grab_input) {
            mvprintw(LINES - 1, 0, ":%-*s", COLS - 1, inp_buf);
        } else {
            move(LINES - 1, 0);
            clrtoeol();
        }

        refresh();

        if (tui->status->updated)
            tui->status->update(tui->status);

        wrefresh(tui->status->win);

        if (tui->sel_window->updated) {
            DEBUG_PRINTF("Update win: %s %d\n", tui->sel_window->win_name, tui->sel_window->updated);
            tui->sel_window->update(tui->sel_window);
        }

        wrefresh(tui->sel_window->win);

        if (tui->sel_window->show_cursor)
            curs_set(1);

        poll(main_notify, sizeof(main_notify)/sizeof(main_notify[0]), tui->sel_window->timeout);

        if (main_notify[0].revents & POLLIN) {
            handle_player_fd(tui, main_notify[0].fd);
            continue;
        }

        if (main_notify[1].revents & POLLIN) {
            handle_notify_fd(tui, main_notify[1].fd);
            continue;
        }

        if (main_notify[2].revents & POLLIN) {
            handle_signal_fd(tui, main_notify[2].fd);
            continue;
        }

        if (main_notify[3].revents & POLLIN) {
            handle_stdin_fd(tui, STDIN_FILENO);
            continue;
        }
    }

    tui->status->clean(tui->status);

    for (win = tui->windows; *win; win++) {
        struct nclyr_win *w = *win;
        if (w->clean)
            w->clean(w);
        delwin(w->win);
    }

    endwin();

    player_state_full_clear(&tui->state);

    return ;
}

struct tui_iface tui_iface  = {
    .iface = {
        .name = "tui",
        .description = "Text (ncurses-based) User Interface",
        .main_loop = tui_main_loop,
    },
    .state = { .is_up = 0 },
    .windows = nclyr_windows,
    .window_count = sizeof(nclyr_windows)/sizeof(*nclyr_windows) - 1,
    .global_keys = (const struct nclyr_keypress[]) {
        N_KEYPRESS('q', global_keys, "Switch to previous window."),
        N_KEYPRESS('w', global_keys, "Switch to next window."),
        N_KEYPRESS('Q', global_keys, "Exit TUI."),
        N_KEYPRESS(':', global_keys, "Start command entry."),

        N_KEYPRESS(' ', player_keys, "Toggle Pause"),
        N_KEYPRESS('p', player_keys, "Previous song"),
        N_KEYPRESS('n', player_keys, "Next song"),
        N_KEYPRESS('+', player_keys, "+1 volume"),
        N_KEYPRESS('-', player_keys, "-1 volume"),

        N_END()
    },
    .show_status = 1,
    .status = &statusline,
    .sel_window_index = 0,
    .sel_window = NULL,
    .exit_flag = 0,
    .grab_input = 0,
};

