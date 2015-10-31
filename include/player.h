#ifndef INCLUDE_PLAYER_H
#define INCLUDE_PLAYER_H

#include "common.h"
#include "config.h"
#include "song.h"
#include "playlist.h"
#include "directory.h"

#include <limits.h>

enum player_state {
    PLAYER_PAUSED,
    PLAYER_PLAYING,
    PLAYER_STOPPED
};

struct player_flags {
    unsigned int is_random :1;
    unsigned int is_single :1;
    unsigned int is_consume :1;
    unsigned int is_crossfade :1;
    unsigned int is_repeat :1;
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
    PLAYER_SONG_POS,
    PLAYER_DIRECTORY,
    PLAYER_FLAGS,
};

struct player_notification {
    enum player_notif_type type;
    union {
        struct song_info *song;
        enum player_state state;
        size_t seek_pos;
        size_t volume;
        struct playlist playlist;
        int song_pos;
        struct directory dir;
        struct player_flags flags;
    } u;
};
STATIC_ASSERT(sizeof(struct player_notification) <= PIPE_BUF);

/* A struct capable of holding all the information recieved from every type of
 * player_notification */
struct player_state_full {
    unsigned int is_up :1;
    enum player_state state;
    struct song_info *song;
    size_t volume;
    size_t seek_pos;
    struct playlist playlist;
    int song_pos;
    struct directory cwd;
    struct player_flags flags;
};

#define PLAYER_STATE_FULL_INIT(state_full) \
    { \
        .is_up = 0, \
        .state = 0, \
        .song = NULL, \
        .volume = 0, \
        .seek_pos = 0, \
        .playlist = PLAYLIST_INIT(), \
        .song_pos = 0, \
        .cwd = DIRECTORY_INIT((state_full).cwd), \
    }

static inline void player_state_full_init(struct player_state_full *state)
{
    memset(state, 0, sizeof(*state));
    playlist_init(&state->playlist);
    directory_init(&state->cwd);
}

enum player_ctrl_msg_type {
    PLAYER_CTRL_PLAY,
    PLAYER_CTRL_PAUSE,
    PLAYER_CTRL_TOGGLE_PAUSE,
    PLAYER_CTRL_NEXT,
    PLAYER_CTRL_PREV,
    PLAYER_CTRL_SEEK,
    PLAYER_CTRL_SET_VOLUME,
    PLAYER_CTRL_CHANGE_VOLUME,
    PLAYER_CTRL_CHANGE_SONG,
    PLAYER_CTRL_REMOVE_SONG,
    PLAYER_CTRL_ADD_SONG,
    PLAYER_CTRL_GET_DIRECTORY,
    PLAYER_CTRL_CHANGE_DIRECTORY,
    PLAYER_CTRL_TOGGLE_FLAGS,
};

struct player_ctrl_msg {
    enum player_ctrl_msg_type type;
    union {
        size_t seek_pos;
        size_t volume;
        int pause;
        int vol_change;
        int song_pos;
        char *change_dir;
        char *song_name;
        struct player_flags flags;
    } u;
};
STATIC_ASSERT(sizeof(struct player_ctrl_msg) <= PIPE_BUF);

struct player;

/* 'has_ctrl_flag' holds bit-flags representing which control's the associated
 * player supports */
struct player_controls {
    void (*ctrl) (struct player *, const struct player_ctrl_msg *);

    uint32_t has_ctrl_flag;
};


struct player {
    const char *name;
    int notify_fd;
    uint32_t notify_flags;

    void (*start_thread) (struct player *);
    void (*stop_thread) (struct player *);

    struct player_controls ctrls;

    /* List of extra windows that should be displayed when using this player */
    const struct nclyr_win **player_windows;
};

/* bit-flags indicating the type of notifications this player is capable of
 * sending. This is important for enabling or disabling certain functionality
 */
enum player_notif_flags {
    PN_HAS_NO_SONG,
    PN_HAS_SONG,
    PN_HAS_IS_UP,
    PN_HAS_IS_DOWN,
    PN_HAS_STATE,
    PN_HAS_SEEK,
    PN_HAS_VOLUME,
    PN_HAS_PLAYLIST,
    PN_HAS_SONG_POS,
    PN_HAS_DIRECTORY,
    PN_HAS_FLAGS,
};

/* bit-flags indicating the type of control commands this player is capable of
 * recieving */
enum player_ctrl_flags {
    PC_HAS_PAUSE,
    PC_HAS_TOGGLE_PAUSE,
    PC_HAS_PLAY,
    PC_HAS_NEXT,
    PC_HAS_PREV,
    PC_HAS_SEEK,
    PC_HAS_SET_VOLUME,
    PC_HAS_CHANGE_VOLUME,
    PC_HAS_CHANGE_SONG,
    PC_HAS_MOVE_SONG,
    PC_HAS_ADD_SONG,
    PC_HAS_GET_DIRECTORY,
    PC_HAS_CHANGE_DIRECTORY,
    PC_HAS_TOGGLE_FLAGS,
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
void player_change_song(struct player *, int song_pos);
void player_remove_song(struct player *, int song_pos);
void player_add_song(struct player *, char *song);
void player_change_working_directory(struct player *, char *dir);
void player_get_working_directory(struct player *);
void player_toggle_flags(struct player *, struct player_flags);

void player_send_is_up(struct player *);
void player_send_is_down(struct player *);
void player_send_state(struct player *, enum player_state);
void player_send_no_song(struct player *);
void player_send_cur_song(struct player *, struct song_info *);
void player_send_seek(struct player *, size_t seek_pos);
void player_send_volume(struct player *, size_t volume);
void player_send_playlist(struct player *, struct playlist *);
void player_send_song_pos(struct player *, int song_pos);
void player_send_directory(struct player *, struct directory *);
void player_send_flags(struct player *, struct player_flags);

enum {
#if CONFIG_PLAYER_MPD
    PLAYER_CONFIG_MPD,
#endif
#if CONFIG_PLAYER_PIANOBAR
    PLAYER_CONFIG_PIANOBAR,
#endif
    PLAYER_CONFIG_TOTAL
};

extern struct config_item players_config[];

#endif
