
#include <stdio.h>

#include "config.h"

void m_begin(void)
{

}

void m_end(void)
{

}

void m_write_var(const char *var, int set)
{
    if (set)
        printf("%s := y\n", var);
    else
        printf("# %s := n\n", var);
}

struct config_output m_output = {
    .start = m_begin,
    .end = m_end,
    .write_var = m_write_var,
};

