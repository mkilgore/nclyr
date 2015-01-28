#ifndef NCLYR_NCLYR_CONF
#define NCLYR_NCLYR_CONF

#include "config.h"

void nclyr_conf_init(void);
void nclyr_conf_clear(void);

struct config_item *nclyr_conf_get(const char *str);

extern struct root_config nclyr_config;

#endif
