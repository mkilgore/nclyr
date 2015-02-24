
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
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "console_cons_str.h"
#include "console.h"
#include "debug.h"

static struct cons_printf_arg args[] = {
    { .id = "title", .type = CONS_ARG_STRING },
    { .id = "artist", .type = CONS_ARG_STRING },
    { .id = "album", .type = CONS_ARG_STRING },
    { .id = "duration", .type = CONS_ARG_TIME },
    { .id = "position", .type = CONS_ARG_TIME },
    { .id = "paused", .type = CONS_ARG_BOOL },
    { .id = "volume", .type = CONS_ARG_INT },
};

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
    cons_printf_compiled *comp;
    struct player_state_full play_state;
    char linebuf[200];
    struct termios oldterm, newterm;
    int cols, i, inp_flag = 0, inp_len = 0, song_change = 0;
    int console_exit_flag = 0;
    struct pollfd main_notify[4];
    struct cons_str chstr;

    memset(&play_state, 0, sizeof(play_state));

    main_notify[0].fd = pipes->player[0];
    main_notify[0].events = POLLIN;

    main_notify[1].fd = pipes->lyr[0];
    main_notify[1].events = POLLIN;

    main_notify[2].fd = pipes->sig[0];
    main_notify[2].events = POLLIN;

    main_notify[3].fd = STDIN_FILENO;
    main_notify[3].events = POLLIN;

    comp = cons_printf_compile(console_conf[CONSOLE_CONFIG_STATUSLINE].u.str.str, ARRAY_SIZE(args), args);

    term_settings(&oldterm, &newterm);

    setvbuf(stdout, NULL, _IONBF, 0);

    while (!console_exit_flag) {
        cols = get_term_width();
        char *buf;

        if (!inp_flag) {
            if (play_state.song) {
                args[0].u.str_val = play_state.song->tag.title;
                args[1].u.str_val = play_state.song->tag.artist;
                args[2].u.str_val = play_state.song->tag.album;
                args[3].u.time_val = play_state.song->duration;
            } else {
                args[0].u.str_val = NULL;
                args[1].u.str_val = NULL;
                args[2].u.str_val = NULL;
                args[3].u.time_val = 0;
            }

            args[4].u.time_val = play_state.seek_pos;
            args[5].u.bool_val = play_state.state == PLAYER_PAUSED;
            args[6].u.int_val = play_state.volume;

            cons_str_init(&chstr);

            cons_printf(comp, &chstr, cols, 0, args, ARRAY_SIZE(args));
            /*
            if (play_state.is_up) {
                if (play_state.state != PLAYER_STOPPED && play_state.song) {
                    char *endline;
                    int len_print;
                    len = snprintf(line, cols, "[%zu%%] %s by %s on %s%s", play_state.volume, play_state.song->tag.title, play_state.song->tag.artist, play_state.song->tag.album, (play_state.state == PLAYER_PAUSED)? " [paused]":"");
                    if (len < cols - 15) {
                        endline = line + len;
                        len_print = cols - len - 15;
                    } else {
                        endline = line + cols - 15;
                        len_print = 0;
                    }
                    snprintf(endline, cols, "%*s[%02zu:%02zu]/[%02zu:%02zu]", len_print, "", play_state.seek_pos / 60, play_state.seek_pos % 60, play_state.song->duration / 60, play_state.song->duration % 60);
                } else {
                    snprintf(line, cols, "Player stopped");
                }
            } else {
                snprintf(line, cols, "Player %s is not open.", player_current()->name);
            } */
            buf = console_cons_str_print(&chstr);
            printf("\r" TERM_CLEAR "%s", buf);

            free(buf);
            cons_str_clear(&chstr);
        } else {
            printf("\r" TERM_CLEAR ":%-.*s", cols - 2, linebuf);
        }


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
                case '?':
                    printf("Console help:\n"
                           " p    - Display playlist\n"
                           "Space - Toggle Pause\n"
                           " >    - Next song\n"
                           " <    - Previous song\n"
                           " +    - Increse volume by 1\n"
                           " -    - Decrease volume by 1\n"
                           " s    - Change song, enter song playlist number at prompt\n"
                          );
                    break;
                case 'p':
                    printf("Playlist:\n");
                    for (i = 0; i < play_state.playlist.song_count; i++) {
                        int eq = (i == play_state.song_pos);
                        printf("%02d. %s%s by %s on %s%s\n",
                                i,
                                (eq)? TERM_COLOR_BLUE:"",
                                play_state.playlist.songs[i]->tag.title,
                                play_state.playlist.songs[i]->tag.artist,
                                play_state.playlist.songs[i]->tag.album,
                                (eq)? TERM_COLOR_RESET:"");
                    }
                    break;
                case ' ':
                    player_toggle_pause(player_current());
                    break;
                case '>':
                    player_next(player_current());
                    break;
                case '<':
                    player_prev(player_current());
                    break;
                case 'P':
                    player_play(player_current());
                    break;
                case '+':
                    player_change_volume(player_current(), 1);
                    break;
                case '-':
                    player_change_volume(player_current(), -1);
                    break;
                case 's':
                    song_change = 1;
                    inp_flag = 1;
                    inp_len = 0;
                    memset(linebuf, 0, sizeof(linebuf));
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
                    inp_flag = 0;
                    if (song_change) {
                        if (inp_len > 0) {
                            int song = strtol(linebuf, NULL, 0);
                            player_change_song(player_current(), song);
                        }
                        song_change = 0;
                    }
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

    cons_printf_compiled_free(comp);
}

struct nclyr_iface console_iface = {
    .name = "console",
    .description = "Console (command-line) Interface",
    .main_loop = console_main_loop,
};

