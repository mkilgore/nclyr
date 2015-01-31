
#include <stdio.h>

#include "config.h"

void m_begin(void)
{

}

void m_end(void)
{

}

void m_write_var(const char *var, int set, const char *dat)
{
    if (set == 1)
        printf("%s := y\n", var);
    else if (set == 0)
        printf("# %s := n\n", var);
    else if (set == 2)
        printf("%s := %s\n", var, dat);
}

struct config_output m_output = {
    .start = m_begin,
    .end = m_end,
    .write_var = m_write_var,
};

