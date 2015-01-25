
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui/window.h"
#include "tui_state.h"
#include "tui_internal.h"
#include "statusline.h"
#include "clock_win.h"
#include "lyrics_win.h"
#include "help_win.h"
#include "artist_win.h"

#include "tui.h"
#include "debug.h"

static struct nclyr_win *nclyr_windows[] = {
    &clock_window.super_win,
#ifdef CONFIG_LIB_GLYR
    &lyrics_window.super_win,
    &artist_window.super_win,
#endif
    &help_window.super_win,
    NULL
};

static int tui_exit_flag = 0;

static void global_keys(struct nclyr_win *, int ch);
static void player_keys(struct nclyr_win *, int ch);

struct tui_state tui = {
    .cur_song = { NULL },
    .windows = nclyr_windows,
    .window_count = sizeof(nclyr_windows)/sizeof(*nclyr_windows) - 1,
    .global_keys = (const struct nclyr_keypress[]) {
        { 'q', global_keys, "Switch to previous window." },
        { 'w', global_keys, "Switch to next window." },

        { ' ', player_keys, "Toggle Pause" },
        { 'p', player_keys, "Previous song" },
        { 'n', player_keys, "Next song" },
        { '+', player_keys, "+1 volume" },
        { '-', player_keys, "-1 volume" },

        { '\0', NULL, NULL }
    },
    .show_status = 1,
    .status = &statusline,
    .sel_window_index = 0,
    .sel_window = NULL,
};

static void player_keys(struct nclyr_win *win, int ch)
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

static void global_keys(struct nclyr_win *win, int ch)
{

    DEBUG_PRINTF("Got global key: %c\n", ch);
    if (ch == 'w') {
        tui.sel_window_index = (tui.sel_window_index + 1) % tui.window_count;
        tui_state_change_window(&tui, tui.windows[tui.sel_window_index]);
    } else if (ch == 'q') {
        tui.sel_window_index = tui.sel_window_index - 1;
        if (tui.sel_window_index == -1)
            tui.sel_window_index = tui.window_count - 1;

        tui_state_change_window(&tui, tui.windows[tui.sel_window_index]);
    }
}

static void handle_player_fd(int playerfd)
{
    struct player_notification notif;
    struct nclyr_win **win;

    read(playerfd, &notif, sizeof(notif));

    DEBUG_PRINTF("Got Player Notification in TUI thread\n");

    if (notif.type == PLAYER_SONG) {
        tui_state_change_song(&tui, &notif.u.song);

        for (win = tui.windows; *win; win++)
            (*win)->already_lookedup = 0;

        tui.sel_window->already_lookedup = 1;
        lyr_thread_song_lookup(&tui.cur_song, tui.sel_window->lyr_types);
    }

    for (win = tui.windows; *win; win++)
        (*win)->new_player_notif(*win, &notif);

    tui.status->player_notif(tui.status, &notif);

    player_notification_free(&notif);
    return ;
}

static void handle_notify_fd(int notifyfd)
{
    struct nclyr_win **win;
    struct lyr_thread_notify song_notif;
    const enum lyr_data_type *song_data;

    read(notifyfd, &song_notif, sizeof(song_notif));

    DEBUG_PRINTF("Got Lyr-thread notification: %d\n", song_notif.type);

    if (strcmp(song_notif.song->title, tui.cur_song.title) != 0
            || strcmp(song_notif.song->artist, tui.cur_song.artist) != 0
            || strcmp(song_notif.song->album, tui.cur_song.album) != 0) {
        DEBUG_PRINTF("Song didn't match!\n");
        goto clear_song_notify;
    }

    for (win = tui.windows; *win; win++)
        for (song_data = (*win)->lyr_types; *song_data != -1; song_data++)
            if (*song_data == song_notif.type)
                (*win)->new_song_data(*win, &song_notif);

clear_song_notify:
    lyr_thread_notify_clear(&song_notif);
    return ;
}

static void handle_signal_fd(int signalfd)
{
    int sig, rows;
    struct nclyr_win **win;

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

        for (win = tui.windows; *win; win++)
            (*win)->resize(*win, rows + 1, 0, LINES - rows - 1, COLS);

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
        tui.sel_window->keypresses,
        NULL
    };

    for (cur_keylist = keylist; *cur_keylist != NULL; cur_keylist++) {
        for (key = *cur_keylist; key->ch != '\0'; key++) {
            if (key->ch == ch) {
                key->callback(tui.sel_window, ch);
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
    struct nclyr_win **win;

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

    for (win = tui.windows; *win; win++)
        (*win)->init(*win, rows + 1, 0, LINES - rows - 1, COLS);

    main_notify[0].fd = pipefd;
    main_notify[0].events = POLLIN;

    main_notify[1].fd = notifyfd;
    main_notify[1].events = POLLIN;

    main_notify[2].fd = signalfd;
    main_notify[2].events = POLLIN;

    main_notify[3].fd = STDIN_FILENO;
    main_notify[3].events = POLLIN;

    tui.sel_window = tui.windows[tui.sel_window_index];

    while (!tui_exit_flag) {
        curs_set(0);

        for (i = 0; i < COLS; i++)
            mvaddch(rows, i, ACS_HLINE);

        move(rows, COLS / 2 - strlen(tui.sel_window->win_name) / 2 - 2);

        addch(ACS_RTEE);
        printw(" %s ", tui.sel_window->win_name);
        addch(ACS_LTEE);

        refresh();

        tui.status->update(tui.status);
        tui.sel_window->update(tui.sel_window);

        if (tui.sel_window->show_cursor)
            curs_set(1);

        poll(main_notify, sizeof(main_notify)/sizeof(main_notify[0]), tui.sel_window->timeout);

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

    for (win = tui.windows; *win; win++)
        (*win)->clean(*win);

    endwin();

    return ;
}

