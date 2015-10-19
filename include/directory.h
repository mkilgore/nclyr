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
    list_node_t dir_entry;
    char *name;
    enum entry_type type;
    struct song_info *song;
};

struct directory {
    char *name;
    int entry_count;
    list_head_t entries;
};

static inline void directory_init(struct directory *dir)
{
    memset(dir, 0, sizeof(*dir));
    INIT_LIST_HEAD(&dir->entries);
}

void directory_clear(struct directory *dir);

static inline void directory_move(struct directory *new, struct directory *old)
{
    DEBUG_PRINTF("Old: %p, %p\n", &old->entries, old->entries.next);
    list_replace(&new->entries, &old->entries);
    DEBUG_PRINTF("New: %p, %p\n", &new->entries, new->entries.next);
    new->name = old->name;
    new->entry_count = old->entry_count;
    directory_init(old);
}

#define DIRECTORY_INIT(dir) \
    { \
        .name = NULL, \
        .entries = LIST_HEAD_INIT(dir.entries), \
        .entry_count = 0, \
    }

void directory_entry_copy(struct directory_entry *new, struct directory_entry *entry);
void directory_entry_clear(struct directory_entry *entry);

#endif
