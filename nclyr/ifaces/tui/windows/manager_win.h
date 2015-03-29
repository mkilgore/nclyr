#ifndef NCLYR_IFACES_TUI_WINDOWS_MANAGER_WIN_H
#define NCLYR_IFACES_TUI_WINDOWS_MANAGER_WIN_H

#include "window.h"
#include "cons/printf.h"

struct manager_win {
    struct nclyr_win super_win;

    int selected;
    int disp_offset;

    cons_printf_compiled *printwin;
};

extern struct manager_win manager_win;

#endif
