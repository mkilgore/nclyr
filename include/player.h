#ifndef INCLUDE_PLAYER_H
#define INCLUDE_PLAYER_H

#include "common.h"
#include "song.h"

#include <limits.h>

struct player;

/* pointers are set to NULL if player doesn't support that feature */
struct player_controls {
    void (*pause) (struct player *);
    void (*play) (struct player *);
    void (*next) (struct player *);
    void (*prev) (struct player *);

    void (*seek) (struct player *, size_t position);

    void (*shuffle) (struct player *);
    void (*set_volume) (struct player *, size_t volume);
};

struct player {
    const char *name;

    void (*start_monitor) (struct player *, int pipe_fd);
    void (*stop_monitor) (struct player *);

    struct player_controls ctrls;
};

struct player_notification {
    enum {
        PLAYER_NO_SONG,
        PLAYER_SONG
    } type;
    union {
        struct song_info song;
    } u;
};
STATIC_ASSERT(sizeof(struct player_notification) <= PIPE_BUF);

extern struct player *players[];
struct player *player_current_used(void);

void player_setup_notification(int pipefd);
void player_stop_notification(void);

void player_notification_free(struct player_notification *);

#endif
