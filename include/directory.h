#ifndef INCLUDE_DIRECTORY_H
#define INCLUDE_DIRECTORY_H

#include <string.h>
#include "list.h"
#include "song.h"
#include "debug.h"

enum entry_type {
    ENTRY_TYPE_DIR,
    ENTRY_TYPE_SONG,
};

struct directory_entry {
    char *name;
    enum entry_type type;
    struct song_info *song;
};

struct directory {
    char *name;
    int entry_count;
    struct directory_entry *entries;
};

static inline void directory_init(struct directory *dir)
{
    memset(dir, 0, sizeof(*dir));
}

void directory_clear(struct directory *dir);

static inline void directory_move(struct directory *new, struct directory *old)
{
    memcpy(new, old, sizeof(*new));
    directory_init(old);
}

#define DIRECTORY_INIT(dir) \
    { \
        .name = NULL, \
        .entries = NULL, \
        .entry_count = 0, \
    }

void directory_entry_copy(struct directory_entry *new, struct directory_entry *entry);
void directory_entry_clear(struct directory_entry *entry);

#endif
