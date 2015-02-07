
#include "common.h"

#include <unistd.h>

#include "stringcasecmp.h"
#include "iface.h"
#include "debug.h"

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

