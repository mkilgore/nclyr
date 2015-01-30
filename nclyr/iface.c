
#include "common.h"

#include <unistd.h>

#include "stringcasecmp.h"
#include "iface.h"
#include "tui.h"
#include "debug.h"

struct nclyr_iface *nclyr_iface_list[] = {
#if CONFIG_TUI
    &tui_iface,
#endif
    NULL
};

struct nclyr_iface *nclyr_iface_find(const char *name)
{
    struct nclyr_iface **iface;
    for (iface = nclyr_iface_list; *iface; iface++)
        if (stringcasecmp((*iface)->name, name) == 0)
            return *iface;

    return NULL;
}

void nclyr_pipes_open(struct nclyr_pipes *pipes)
{
    pipe(pipes->sig);
    pipe(pipes->player);
    pipe(pipes->lyr);
}

void nclyr_pipes_close(struct nclyr_pipes *pipes)
{
    close(pipes->sig[0]);
    close(pipes->sig[1]);
    close(pipes->player[0]);
    close(pipes->player[1]);
    close(pipes->lyr[0]);
    close(pipes->lyr[1]);
}

