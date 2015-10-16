
#include "common.h"

#include <string.h>

#include "song.h"
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "printf/compiler.h"
#include "arg_song.h"
#include "debug.h"

struct printf_opt_arg_song {
    struct printf_opt opt;
    int arg;
};

static struct cons_printf_arg song_args[] = {
    { .id = "title", .type = CONS_ARG_STRING },
    { .id = "artist", .type = CONS_ARG_STRING },
    { .id = "album", .type = CONS_ARG_STRING },
    { .id = "duration", .type = CONS_ARG_TIME },
    { .id = "filename", .type = CONS_ARG_STRING },
    { .id = "playing", .type = CONS_ARG_BOOL },
    { .id = "selected", .type = CONS_ARG_BOOL },
};
/*
static cons_printf_compiled *triple_format, *filename_format;

void cons_printf_set_song_triple(const char *format)
{
    if (triple_format)
        cons_printf_compiled_free(triple_format);

    triple_format = cons_printf_compile(format, ARRAY_SIZE(song_args), song_args);
}

void cons_printf_set_song_filename(const char *format)
{
    if (filename_format)
        cons_printf_compiled_free(filename_format);

    filename_format = cons_printf_compile(format, ARRAY_SIZE(song_args), song_args);
}
*/

cons_printf_compiled *cons_printf_compile_song(const char *format)
{
    return cons_printf_compile(format, ARRAY_SIZE(song_args), song_args);
}

static void print_arg_song(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_song *arg = container_of(opt, struct printf_opt_arg_song, opt);
    struct song_info *song = args[arg->arg].u.song.s;

    if (song) {
        struct cons_str result;

        song_args[0].u.str_val = song->tag.title;
        song_args[1].u.str_val = song->tag.artist;
        song_args[2].u.str_val = song->tag.album;
        song_args[3].u.int_val = song->duration;
        song_args[4].u.str_val = song->name;

        if (song->tag.title && song->tag.artist && song->tag.album)
            cons_printf(args[arg->arg].u.song.triple, &result, chstr->max_width, CONS_PRINTF_COMP_ATTRS(comp), song_args, ARRAY_SIZE(song_args));
        else
            cons_printf(args[arg->arg].u.song.filename, &result, chstr->max_width, CONS_PRINTF_COMP_ATTRS(comp), song_args, ARRAY_SIZE(song_args));

        cons_str_add_cons_str(chstr, &result);
        cons_str_clear(&result);
    }
}

struct printf_opt *printf_arg_parse_song(int index, char *id_par, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_song *arg = malloc(sizeof(*arg));
    memset(arg, 0, sizeof(*arg));
    arg->opt.print = print_arg_song;
    arg->opt.clear = printf_opt_free;
    arg->arg = index;
    return &arg->opt;
}

