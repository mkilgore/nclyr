
#include "common.h"

#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "song.h"
#include "playlist.h"
#include "player.h"
#include "iface.h"
#include "termcols.h"
#include "cmd.h"
#include "console.h"
#include "debug.h"

static int get_term_width(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

static void term_settings(struct termios *old, struct termios *new)
{
    tcgetattr(STDIN_FILENO, old);
    memcpy(new, old, sizeof(*new));
    new->c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, new);
    return ;
}

static void console_main_loop(struct nclyr_iface *iface, struct nclyr_pipes *pipes)
{
    struct player_state_full play_state;
    char linebuf[200];
    struct termios oldterm, newterm;
    int cols, i, inp_flag = 0, inp_len = 0;
    int console_exit_flag = 0;
    struct pollfd main_notify[4];

    memset(&play_state, 0, sizeof(play_state));

    main_notify[0].fd = pipes->player[0];
    main_notify[0].events = POLLIN;

    main_notify[1].fd = pipes->lyr[0];
    main_notify[1].events = POLLIN;

    main_notify[2].fd = pipes->sig[0];
    main_notify[2].events = POLLIN;

    main_notify[3].fd = STDIN_FILENO;
    main_notify[3].events = POLLIN;

    term_settings(&oldterm, &newterm);

    setvbuf(stdout, NULL, _IONBF, 0);

    while (!console_exit_flag) {
        int len;
        cols = get_term_width();
        char line[cols + 1];

        if (!inp_flag) {
            if (play_state.is_up) {
                if (play_state.state != PLAYER_STOPPED && play_state.song) {
                    len = snprintf(line, cols, "[%d%%] Song: %s by %s on %s%s", play_state.volume, play_state.song->title, play_state.song->artist, play_state.song->album, (play_state.state == PLAYER_PAUSED)? " [paused]":"");
                    if (len < cols)
                        snprintf(line + len, cols, "%*s[%02d:%02d]/[%02d:%02d]", cols - len - 15, "", play_state.seek_pos / 60, play_state.seek_pos % 60, play_state.song->duration / 60, play_state.song->duration % 60);
                } else {
                    snprintf(line, cols, "Player stopped");
                }
            } else {
                snprintf(line, cols, "Player %s is not open.", player_current()->name);
            }
        } else {
            snprintf(line, cols, ":%-*s", cols - 1, linebuf);
        }


        printf("\r" TERM_CLEAR "%s", line);

        poll(main_notify, sizeof(main_notify)/sizeof(*main_notify), -1);

        printf("\r" TERM_CLEAR);

        if (main_notify[0].revents & POLLIN) {
            struct player_notification notif;
            read(main_notify[0].fd, &notif, sizeof(notif));
            player_state_full_update(&play_state, &notif);
            player_notification_clear(&notif);
            continue;
        }

        if (main_notify[1].revents & POLLIN) {
            continue;
        }

        if (main_notify[2].revents & POLLIN) {
            int sig;
            read(main_notify[2].fd, &sig, sizeof(sig));
            if (sig == SIGINT)
                console_exit_flag = 1;
            continue;
        }

        if (main_notify[3].revents & POLLIN) {
            char ch = getchar();
            if (!inp_flag) {
                switch (ch) {
                case 'h':
                    printf("Console help:\nstuff\nstuff\n");
                    break;
                case 'p':
                    printf("Playlist:\n");
                    for (i = 0; i < play_state.playlist.song_count; i++) {
                        int eq = song_equal(play_state.playlist.songs[i], play_state.song);
                        printf("%02d. %s%s by %s on %s%s\n",
                                i,
                                (eq)? TERM_COLOR_BLUE:"",
                                play_state.playlist.songs[i]->title,
                                play_state.playlist.songs[i]->artist,
                                play_state.playlist.songs[i]->album,
                                (eq)? TERM_COLOR_RESET:"");
                    }
                    break;
                case ' ':
                    player_toggle_pause(player_current());
                    break;
                case ':':
                    printf("\r" TERM_CLEAR ":");
                    inp_flag = 1;
                    inp_len = 0;
                    memset(linebuf, 0, sizeof(linebuf));
                    break;
                }
            } else {
                if (ch == '\n') {
                    printf("Input: %s\n", linebuf);
                    inp_flag = 0;
                } else if (ch != 127) {
                    if (inp_len < sizeof(linebuf))
                        linebuf[inp_len++] = (char)ch;
                } else {
                    if (inp_len > 0)
                        linebuf[--inp_len] = '\0';
                }
            }
            continue;
        }
    }

    player_state_full_clear(&play_state);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
}

struct nclyr_iface console_iface = {
    .name = "console",
    .description = "Console (command-line) Interface",
    .main_loop = console_main_loop,
};

