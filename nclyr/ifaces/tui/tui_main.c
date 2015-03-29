
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
#include "cmd_exec.h"
#include "windows/window.h"

#include "tui.h"
#include "debug.h"

static void handle_player_fd(struct tui_iface *tui, int playerfd)
{
    struct player_notification notif;
    int i;

    read(playerfd, &notif, sizeof(notif));

    DEBUG_PRINTF("Got Player Notification in TUI thread\n");

    player_state_full_update(&tui->state, &notif);

    if (notif.type == PLAYER_SONG) {
        for (i = 0; i < tui->window_count; i++) {
            struct nclyr_win *win = tui->windows[i];

            if (win->clear_song_data)
                win->clear_song_data(win);
            win->already_lookedup = 0;
        }

        tui->sel_window->already_lookedup = 1;
        if (tui->state.song)
            lyr_thread_song_lookup(tui->state.song, tui->sel_window->lyr_types);
    }

    for (i = 0; i < tui->window_count; i++) {
        struct nclyr_win *win = tui->windows[i];

        if (win->new_player_notif)
            win->new_player_notif(win, notif.type, &tui->state);
    }

    tui->status->player_notif(tui->status, notif.type, &tui->state);

    player_notification_clear(&notif);
    return ;
}

static void handle_notify_fd(struct tui_iface *tui, int notifyfd)
{
    int i;
    struct lyr_thread_notify song_notif;
    const enum lyr_data_type *song_data;

    read(notifyfd, &song_notif, sizeof(song_notif));

    DEBUG_PRINTF("Got Lyr-thread notification: %d\n", song_notif.type);

    if (!song_equal(song_notif.song, tui->state.song)) {
        DEBUG_PRINTF("Song didn't match!\n");
        goto clear_song_notify;
    }

    for (i = 0; i < tui->window_count; i++) {
        struct nclyr_win *win = tui->windows[i];
        for (song_data = win->lyr_types; *song_data != LYR_DATA_TYPE_COUNT; song_data++)
            if (*song_data == song_notif.type)
                win->new_song_data(win, &song_notif);
    }

clear_song_notify:
    lyr_thread_notify_clear(&song_notif);
    return ;
}

static void handle_signal_fd(struct tui_iface *tui, int signalfd)
{
    int sig, rows, i;

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

        for (i = 0; i < tui->window_count; i++) {
            struct nclyr_win *w = tui->windows[i];

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

/* For whatever reason, ncurses mouse handling seems to be somewhat broken. The
 * below code is ugly for that reason. Perhaps someone who knows what's wrong
 * can fix it. */
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

    /* Either this function is bugged or the documentation s bugged. It says
     * that sending it TRUE should allow us to convert (y, x) to
     * window-relative cords, but I've only gotten it to do so if I send it
     * FALSE instead. */
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
        DEBUG_PRINTF("Button1 Pressed\n");
        mevent.type = LEFT_PRESSED;
        break;

    /* Button4 is actually the scroll wheel, but I've seen a bug that a get
     * BUTTON4_RELEASED in place of a BUTTON1_RELEASED, causing us to lose the
     * left click */
    case BUTTON4_RELEASED:
    case BUTTON1_RELEASED:
        DEBUG_PRINTF("Button1 Released\n");
        if (tui->last_mevent == LEFT_PRESSED)
            mevent.type = LEFT_CLICKED;
        else
            mevent.type = LEFT_RELEASED;
        break;
    case BUTTON3_PRESSED:
        DEBUG_PRINTF("Button3 Pressed\n");
        mevent.type = RIGHT_PRESSED;
        break;
    case BUTTON3_RELEASED:
        DEBUG_PRINTF("Button3 Released\n");
        if (tui->last_mevent == RIGHT_PRESSED)
            mevent.type = RIGHT_CLICKED;
        else
            mevent.type = RIGHT_RELEASED;
        break;
    case BUTTON4_PRESSED:
        DEBUG_PRINTF("Button4 Pressed\n");
        mevent.type = SCROLL_UP;
        break;
    case BUTTON2_PRESSED:
        DEBUG_PRINTF("Button2 Pressed\n");
        mevent.type = SCROLL_DOWN;
        break;

    /* We mask out REPORT_MOUSE_POSITION, but they're still sent when we
     * scroll. They seem to only be sent on duplicates, so we just repeat the
     * last event */
    case REPORT_MOUSE_POSITION:
        DEBUG_PRINTF("Report mouse position\n");
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
            /* If they try to backspace past the beginning of the input, then
             * we exit command input */
            if (strlen(tui->inp_buf) > 0)
                tui->inp_buf[strlen(tui->inp_buf) - 1] = '\0';
            else
                tui->grab_input = 0;
        } else if (ch == KEY_ENTER || ch == '\n') {
            tui->grab_input = 0;
            /* We don't try running the command if the buffer is empty */
            if (strlen(tui->inp_buf))
                tui_cmd_exec(tui, tui->cmds, tui->inp_buf);
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

void tui_main_loop(struct nclyr_iface *iface, struct nclyr_pipes *pipes)
{
    int i = 0;
    int rows;
    char inp_buf[200];
    struct tui_iface *tui = container_of(iface, struct tui_iface, iface);
    struct pollfd main_notify[4];
    struct tui_window_desc *descs[] = { window_descs + 0, window_descs + 1, window_descs + 2, window_descs + 4, NULL };
    struct tui_window_desc **d;

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
    tui->display = NULL;

    tui->cfg = tui_config_get_root();

    tui->status->tui = tui;
    tui->status->init(tui->status, COLS);

    tui_window_init(tui, tui->manager_win);
    tui->manager_win->init(tui->manager_win);

    tui_window_init(tui, tui->help_win);
    tui->help_win->init(tui->help_win);

    rows = getmaxy(tui->status->win);

    for (d = descs; *d; d++)
        tui_window_add(tui, tui_window_new(tui, *d));

    /*
    for (win = tui->windows; *win; win++) {
        struct nclyr_win *w = *win;
        w->tui = tui;
        w->win = newwin(LINES - rows - 2, COLS, rows + 1, 0);
        touchwin(w->win);
        w->updated = 1;

        if (w->init)
            w->init(w);
    } */

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

        if (tui->grab_input)
            mvprintw(LINES - 1, 0, ":%-*s", COLS - 1, inp_buf);
        else
            mvprintw(LINES - 1, 0, "%-*s", COLS, (tui->display)? tui->display: "");

        refresh();

        if (tui->status->updated)
            tui->status->update(tui->status);

        wrefresh(tui->status->win);

        if (tui->sel_window->updated) {
            DEBUG_PRINTF("Update win: %s %d\n", tui->sel_window->win_name, tui->sel_window->updated);
            tui->sel_window->update(tui->sel_window);
        }

        wrefresh(tui->sel_window->win);

        /* We check if the currently selected window wants to show the cursor,
         * and display it if it does.
         *
         * Alternatively, if we're currently getting input for a command, then
         * we want to show the cursor down on the input line at the end of the
         * line. */
        if (tui->sel_window->show_cursor && !tui->grab_input) {
            curs_set(1);
        } else if (tui->grab_input) {
            move(LINES - 1, strlen(tui->inp_buf) + 1);
            refresh();
            curs_set(1);
        }


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
    tui->manager_win->clean(tui->manager_win);
    tui->help_win->clean(tui->help_win);

    for (i = 0; i < tui->window_count; i++) {
        struct nclyr_win *w = tui->windows[i];
        if (w->clean)
            w->clean(w);
        delwin(w->win);
        free(w);
    }

    free(tui->windows);

    endwin();

    player_state_full_clear(&tui->state);

    return ;
}

