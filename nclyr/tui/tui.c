
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "player.h"
#include "window.h"
#include "tui_state.h"
#include "tui_internal.h"
#include "statusline.h"
#include "clock_win.h"
#include "lyrics_win.h"

static struct nclyr_win *nclyr_windows[] = {
    &clock_window.super_win,
    &lyrics_window.super_win,
};

static int tui_exit_flag = 0;

static void global_keys(struct nclyr_win *, int ch);

struct tui_state tui = {
    .cur_song = { NULL },
    .windows = nclyr_windows,
    .window_count = sizeof(nclyr_windows)/sizeof(nclyr_windows[0]),

    .global_keys = (const struct nclyr_keypress[]) {
        { 'q', global_keys, NULL },
        { 'w', global_keys, NULL },
        { '\0', NULL, NULL }
    },
    .show_status = 1,
    .status = &statusline,
    .sel_window = 0
};

static void global_keys(struct nclyr_win *win, int ch)
{

    if (ch == 'w') {
        tui_state_change_window(&tui, (tui.sel_window + 1) % tui.window_count);
    } else if (ch == 'q') {
        int sel = tui.sel_window - 1;
        if (sel == -1)
            sel = tui.window_count - 1;

        tui_state_change_window(&tui, sel);
    }
}

static void handle_player_fd(int playerfd)
{
    int i;
    struct player_notification notif;
    read(playerfd, &notif, sizeof(notif));

    if (notif.type == PLAYER_SONG) {
        tui_state_change_song(&tui, &notif.u.song);

        for (i = 0; i < tui.window_count; i++)
            tui.windows[i]->already_lookedup = 0;

        tui.windows[tui.sel_window]->already_lookedup = 1;
        song_thread_song_lookup(&tui.cur_song, tui.windows[tui.sel_window]->types);
    }
    player_notification_free(&notif);
    return ;
}

static void handle_notify_fd(int notifyfd)
{
    int i = 0;
    struct song_thread_notify song_notif;
    const enum song_data_type *song_data;

    read(notifyfd, &song_notif, sizeof(song_notif));

    if (strcmp(song_notif.song->title, tui.cur_song.title) != 0
            || strcmp(song_notif.song->artist, tui.cur_song.artist) != 0
            || strcmp(song_notif.song->album, tui.cur_song.album) != 0)
        goto clear_song_notify;

    for (i = 0; i < tui.window_count; i++)
        for (song_data = tui.windows[i]->types; *song_data != -1; song_data++)
            if (*song_data == song_notif.type)
                tui.windows[i]->new_song_data(tui.windows[i], &song_notif);

clear_song_notify:
    song_thread_notify_clear(&song_notif);
    return ;
}

static void handle_signal_fd(int signalfd)
{
    int sig, i, rows;

    rows = getmaxy(tui.status->win);

    read(signalfd, &sig, sizeof(sig));
    switch (sig) {
    case SIGINT:
        tui_exit_flag = 1;
        break;
    case SIGWINCH:
        /* endwin() and refresh() are called to force ncurses to resize
         * stdscr and display the new window. */
        endwin();
        refresh();

        for (i = 0; i < tui.window_count; i++)
            tui.windows[i]->resize(tui.windows[i], rows, 0, LINES - rows, COLS);

        tui.status->resize(tui.status, COLS);
        break;
    }
    return ;
}

static void handle_stdin_fd(int stdinfd)
{
    int ch = getch();
    const struct nclyr_keypress *key, **cur_keylist;
    const struct nclyr_keypress *keylist[] = {
        tui.global_keys,
        tui.windows[tui.sel_window]->keypresses,
        NULL
    };

    for (cur_keylist = keylist; *cur_keylist != NULL; cur_keylist++) {
        for (key = *cur_keylist; key->ch != '\0'; key++) {
            if (key->ch == ch) {
                key->callback(tui.windows[tui.sel_window], ch);
                goto found_key;
            }
        }

    }

found_key:
    return ;
}

void tui_main_loop(int signalfd, int pipefd, int notifyfd)
{
    int i = 0;
    int rows;
    struct pollfd main_notify[4];

    initscr();
    cbreak();
    keypad(stdscr, 1);
    nodelay(stdscr, TRUE);
    noecho();
    start_color();
    use_default_colors();

    for (i = 0; i < 8; i++)
        init_pair(i + 1, i, -1);

    tui.status->init(tui.status, COLS);

    rows = getmaxy(tui.status->win);

    for (i = 0; i < tui.window_count; i++)
        tui.windows[i]->init(tui.windows[i], rows, 0, LINES - rows, COLS);

    main_notify[0].fd = pipefd;
    main_notify[0].events = POLLIN;

    main_notify[1].fd = notifyfd;
    main_notify[1].events = POLLIN;

    main_notify[2].fd = signalfd;
    main_notify[2].events = POLLIN;

    main_notify[3].fd = STDIN_FILENO;
    main_notify[3].events = POLLIN;

    while (!tui_exit_flag) {
        tui.status->update(tui.status);
        tui.windows[tui.sel_window]->update(tui.windows[tui.sel_window]);
        refresh();

        poll(main_notify, sizeof(main_notify)/sizeof(main_notify[0]), tui.windows[tui.sel_window]->timeout);

        if (main_notify[0].revents & POLLIN) {
            handle_player_fd(pipefd);
            continue;
        }

        if (main_notify[1].revents & POLLIN) {
            handle_notify_fd(notifyfd);
            continue;
        }

        if (main_notify[2].revents & POLLIN) {
            handle_signal_fd(signalfd);
            continue;
        }

        if (main_notify[3].revents & POLLIN) {
            handle_stdin_fd(STDIN_FILENO);
            continue;
        }
    }

    tui.status->clean(tui.status);

    for (i = 0; i < tui.window_count; i++)
        tui.windows[i]->clean(tui.windows[i]);

    endwin();

    return ;
}

