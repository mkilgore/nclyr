#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H

#include "config.h"
#include "iface.h"

extern struct nclyr_iface console_iface;

enum console_config {
    CONSOLE_CONFIG_STATUSLINE,
    CONSOLE_CONFIG_TOTAL
};

extern struct config_item console_conf[];

#endif
