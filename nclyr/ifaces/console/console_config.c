
#include "common.h"

#include "config.h"
#include "console.h"
#include "debug.h"

struct config_item console_conf[] = {
    [CONSOLE_CONFIG_STATUSLINE] = { .name = "statusline", .type = CONFIG_STRING,
        .u.str = {
            .str = CONFIG_CONSOLE_STATUSLINE_DEFAULT
        },
    }
};


