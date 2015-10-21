
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "build.h"
#include "debug.h"

void directory_clear(struct directory *dir)
{
    int i;

    free(dir->name);

    for (i = 0; i < dir->entry_count; i++)
        directory_entry_clear(dir->entries + i);

    free(dir->entries);
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

