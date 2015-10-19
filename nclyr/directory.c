
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "build.h"
#include "debug.h"

void directory_clear(struct directory *dir)
{
    struct directory_entry *next_entry, *entry;

    free(dir->name);

    next_entry = list_first_entry(&dir->entries, struct directory_entry, dir_entry);

    while (!list_ptr_is_head(&dir->entries, &next_entry->dir_entry)) {
        entry = next_entry;
        next_entry = list_next_entry(entry, struct directory_entry, dir_entry);

        directory_entry_clear(entry);
        free(entry);
    }
}

void directory_entry_copy(struct directory_entry *new, struct directory_entry *entry)
{
    memset(new, 0, sizeof(*new));

    new->type = entry->type;
    if (entry->name)
        new->name = strdup(entry->name);

    if (entry->song)
        new->song = song_copy(entry->song);
}

void directory_entry_clear(struct directory_entry *entry)
{
    free(entry->name);
    song_free(entry->song);
}

