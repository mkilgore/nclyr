
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

static void term_input(struct termios *new, char *buf, size_t bufsize)
{
    new->c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, new);

    fgets(buf, bufsize, stdin);

    new->c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, new);
}

static void console_main_loop(struct nclyr_iface *iface, struct nclyr_pipes *pipes)
{
    char linebuf[200];
    struct termios oldterm, newterm;
    int cols, elapsed = 0;
    int volume = 0, i;
    int console_exit_flag = 0;
    struct pollfd main_notify[4];
    struct song_info cur_song;
    enum player_state state = PLAYER_STOPPED;
    struct playlist playlist = { .song_count = 0, .songs = NULL };

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

    song_init(&cur_song);

    while (!console_exit_flag) {
        int len;
        cols = get_term_width();
        char line[cols + 1];

        if (state != PLAYER_STOPPED)
            len = sprintf(line, "[%d%%] Song: %s by %s on %s%s", volume, cur_song.title, cur_song.artist, cur_song.album, (state == PLAYER_PAUSED)? " [paused]":"");
        else
            len = sprintf(line, "Player stopped");

        sprintf(line + len, "%*s[%02d:%02d]/[%02d:%02d]", cols - len - 15, "", elapsed / 60, elapsed % 60, cur_song.duration / 60, cur_song.duration % 60);

        printf("\r" TERM_CLEAR "%s", line);

        poll(main_notify, sizeof(main_notify)/sizeof(*main_notify), -1);

        printf("\r" TERM_CLEAR);

        if (main_notify[0].revents & POLLIN) {
            struct player_notification notif;
            read(main_notify[0].fd, &notif, sizeof(notif));
            switch (notif.type) {
            case PLAYER_SONG:
                song_clear(&cur_song);
                song_copy(&cur_song, &notif.u.song);
                break;
            case PLAYER_STATE:
                state = notif.u.state;
                break;
            case PLAYER_SEEK:
                elapsed = notif.u.seek_pos;
                break;
            case PLAYER_VOLUME:
                volume = notif.u.volume;
                break;
            case PLAYER_PLAYLIST:
                playlist_clear(&playlist);

                /* Steal the playlist from the notification, instead of making
                 * a copy */
                playlist = notif.u.playlist;
                notif.u.playlist.song_count = 0;
                notif.u.playlist.songs = NULL;
                break;
            default:
                break;
            }

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
            switch (ch) {
            case 'h':
                printf("Console help:\nstuff\nstuff\n");
                break;
            case 'p':
                printf("Playlist:\n");
                for (i = 0; i < playlist.song_count; i++)
                    printf("%02d. %s by %s on %s%s\n",
                            i,
                            playlist.songs[i].title,
                            playlist.songs[i].artist,
                            playlist.songs[i].album,
                            song_equal(playlist.songs + i, &cur_song)? " ***":"");
                break;
            case ' ':
                player_toggle_pause(player_current());
                break;
            case ':':
                printf("\r" TERM_CLEAR ":");
                term_input(&newterm, linebuf, sizeof(linebuf));
                break;
            }
            continue;
        }
    }

    song_clear(&cur_song);
    playlist_clear(&playlist);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
}

struct nclyr_iface console_iface = {
    .name = "console",
    .description = "Console (command-line) Interface",
    .main_loop = console_main_loop,
};

