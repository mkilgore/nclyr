#ifndef INCLUDE_PLAYER_H
#define INCLUDE_PLAYER_H

#include "common.h"
#include "song.h"
#include "playlist.h"

#include <limits.h>

enum player_state {
    PLAYER_PAUSED,
    PLAYER_PLAYING,
    PLAYER_STOPPED
};

enum player_notif_type {
    PLAYER_NO_SONG,
    PLAYER_SONG,
    PLAYER_IS_UP,
    PLAYER_IS_DOWN,
    PLAYER_STATE,
    PLAYER_SEEK,
    PLAYER_VOLUME,
    PLAYER_PLAYLIST,
};

struct player_notification {
    enum player_notif_type type;
    union {
        struct song_info *song;
        enum player_state state;
        size_t seek_pos;
        size_t volume;
        struct playlist playlist;
    } u;
};
STATIC_ASSERT(sizeof(struct player_notification) <= PIPE_BUF);

/* A struct capable of holding all the information recieved from every type of
 * player_notification */
struct player_state_full {
    int is_up :1;
    enum player_state state;
    struct song_info *song;
    size_t volume;
    size_t seek_pos;
    struct playlist playlist;
};

enum player_ctrl_msg_type {
    PLAYER_CTRL_PLAY,
    PLAYER_CTRL_PAUSE,
    PLAYER_CTRL_TOGGLE_PAUSE,
    PLAYER_CTRL_NEXT,
    PLAYER_CTRL_PREV,
    PLAYER_CTRL_SEEK,
    PLAYER_CTRL_SHUFFLE,
    PLAYER_CTRL_SET_VOLUME,
    PLAYER_CTRL_CHANGE_VOLUME,
};

struct player_ctrl_msg {
    enum player_ctrl_msg_type type;
    union {
        size_t seek_pos;
        size_t volume;
        int pause;
        int vol_change;
    } u;
};
STATIC_ASSERT(sizeof(struct player_ctrl_msg) <= PIPE_BUF);

struct player;

/* pointers are set to NULL if player doesn't support that feature */
struct player_controls {
    void (*ctrl) (struct player *, const struct player_ctrl_msg *);

    unsigned int has_pause :1;
    unsigned int has_toggle_pause :1;
    unsigned int has_play :1;
    unsigned int has_next :1;
    unsigned int has_prev :1;
    unsigned int has_seek :1;
    unsigned int has_shuffle :1;
    unsigned int has_set_volume :1;
    unsigned int has_change_volume :1;
};

struct player {
    const char *name;
    int notify_fd;

    void (*start_thread) (struct player *);
    void (*stop_thread) (struct player *);

    struct player_controls ctrls;

    /* List of extra windows that should be displayed when using this player */
    const struct nclyr_win **player_windows;
};


extern struct player *players[];
struct player *player_current(void);

void player_start_thread(struct player *player, int pipefd);
void player_stop_thread(struct player *player);

void player_notification_clear(struct player_notification *);

void player_state_full_clear(struct player_state_full *state);
void player_state_full_update(struct player_state_full *state, struct player_notification *notif);

struct player *player_find(const char *name);
void player_set_current(struct player *);

void player_pause(struct player *, int pause);
void player_toggle_pause(struct player *);
void player_play(struct player *);
void player_next(struct player *);
void player_prev(struct player *);
void player_seek(struct player *, size_t pos);
void player_shuffle(struct player *);
void player_set_volume(struct player *, size_t volume);
void player_change_volume(struct player *, int change);

void player_send_is_up(struct player *);
void player_send_is_down(struct player *);
void player_send_state(struct player *, enum player_state);
void player_send_no_song(struct player *);
void player_send_cur_song(struct player *, struct song_info *);
void player_send_seek(struct player *, size_t seek_pos);
void player_send_volume(struct player *, size_t volume);
void player_send_playlist(struct player *, struct playlist *);

#endif
