
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stringcasecmp.h"
#include "config_lexer.h"
#include "config.h"
#include "debug.h"

void config_item_data_clear(enum config_item_type type, union config_data *data)
{
    int i;
    switch (type) {
    case CONFIG_STRING:
        free(data->str);
        break;

    case CONFIG_GROUP:
        for (i = 0; i < data->group.item_count; i++)
            config_item_clear(data->group.items + i);
        free(data->group.items);
        break;

    case CONFIG_COLOR_PAIR:
    case CONFIG_BOOL:
    case CONFIG_INTEGER:
        break;
    }
}

void config_item_clear(struct config_item *item)
{
    config_item_data_clear(item->type, &item->u);
}

struct config_item *config_item_find(struct root_config *root, const char *id)
{
    int i;
    struct item_group *group;
    char *id2;
    int len;

    group = &root->group;

    DEBUG_PRINTF("Id: %s\n", id);

    do {
        id2 = strchr(id, '-');

        len = id2 - id;

        if (id2 == NULL)
            len = strlen(id);

        for (i = 0; i < group->item_count; i++) {
            struct config_item *item2 = group->items + i;
            DEBUG_PRINTF("Item: %p\n", item2);
            DEBUG_PRINTF("Item Name: %s\n", item2->name);
            if (strncmp(item2->name, id, len) == 0) {
                if (id2 == NULL) {
                    if (item2->type != CONFIG_GROUP)
                        return item2;
                    else
                        return NULL; /* We tried to set the data of a group */
                } else {
                    if (item2->type == CONFIG_GROUP)
                        group = &item2->u.group;
                    else
                        return NULL;
                }
            }
        }

        id = id2 + 1;
    } while (id2 != NULL);
    return NULL;
}

