
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "a_sprintf.h"
#include "tui.h"
#include "tui_internal.h"
#include "cmd_exec.h"
#include "debug.h"

static void add_arg(int *argc, char ***argv, char *new)
{
    (*argc)++;
    *argv = realloc(*argv, sizeof(**argv) * *argc);
    (*argv)[*argc - 1] = new;
}

static void cmd_parse(char *cmd, int *argc, char ***argv)
{
    char *last = NULL;

    *argc = 0;
    *argv = NULL;

    for (; *cmd; cmd++) {
        if (*cmd != ' ' && !last) {
            last = cmd;
        } else if (*cmd == ' ' && last) {
            *cmd = '\0';
            add_arg(argc, argv, last);
            last = NULL;
        }
    }

    if (last)
        add_arg(argc, argv, last);
}

void tui_cmd_exec_args(struct tui_iface *tui, const struct tui_cmd *cmds, int argc, char **argv)
{
    const struct tui_cmd *cmd;
    char *c = argv[0];

    argc--;
    argv++;

    for (cmd = cmds; cmd->cmd; cmd++) {
        DEBUG_PRINTF("Cmd: %s\n", cmd->cmd);
        if (strcmp(c, cmd->cmd) == 0) {
            DEBUG_PRINTF("Found cmd: %s\n", c);
            (cmd->callback) (tui, argc, argv);
            break;
        }
    }

    if (!cmd->cmd)
        a_sprintf(&tui->display, "Unknown comamnd: %s", c);
}

void tui_cmd_exec(struct tui_iface *tui, const struct tui_cmd *cmds, char *cmd)
{
    int argc;
    char **argv;

    cmd_parse(cmd, &argc, &argv);
    tui_cmd_exec_args(tui, cmds, argc, argv);

    free(argv);
}

