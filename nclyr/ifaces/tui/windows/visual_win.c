
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <fcntl.h>
#include <math.h>
#include <fftw3.h>

#include "a_sprintf.h"
#include "config.h"
#include "song.h"
#include "directory.h"
#include "player.h"
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "tui_internal.h"
#include "tui_state.h"
#include "tui_color.h"
#include "window.h"
#include "browse_win.h"
#include "selectable_win.h"
#include "debug.h"

#define VIZ_FOR_COLOR COLOR_WHITE
#define SAMPLES 4096

struct visual_win {
    struct nclyr_win win;

    int vis_fd;
    int f_results, *freq_magnitudes;
    double *f_input;
    fftw_complex *f_output;
    fftw_plan f_plan;
};

static void draw_wave(struct visual_win *vis, int16_t *buf, ssize_t samples, size_t yoff, size_t height)
{
    const int samples_per_col = samples / COLS;
    const int half_height = height / 2;
    double prev_point_pos = 0;
    const size_t win_width = COLS;

    for (size_t i = 0; i < win_width; i++) {
        double point_pos = 0;
        for (int j = 0; j < samples_per_col; j++)
            point_pos += buf[i * samples_per_col + j];
        point_pos /= samples_per_col;
        point_pos /= INT16_MAX;
        point_pos *= half_height;
        mvwprintw(vis->win.win, yoff + half_height + point_pos, i, "*");
        if (i && abs(prev_point_pos - point_pos) > 2) {
            const int breakpoint = (prev_point_pos > point_pos)? prev_point_pos: point_pos;
            const int half = (prev_point_pos + point_pos) / 2;
            for (int k = ((prev_point_pos > point_pos)? point_pos: prev_point_pos) + 1;
                    k < breakpoint;
                    k += 2)
                mvwprintw(vis->win.win, yoff + half_height + k, i - (k < half), "*");
        }
        prev_point_pos = point_pos;
    }
}

static void draw_spectr(struct visual_win *vis, int16_t *buf, ssize_t samples, size_t yoff, size_t height)
{
    for (unsigned int i = 0, j = 0; i < SAMPLES; i++)
        if (j < samples)
            vis->f_input[i] = buf[j++];
        else
            vis->f_input[i] = 0;

    fftw_execute(vis->f_plan);

    for (unsigned int i = 0; i < vis->f_results; i++)
        vis->freq_magnitudes[i] = sqrt(vis->f_output[i][0] * vis->f_output[i][0]
                + vis->f_output[i][1] * vis->f_output[i][1]) / 1e5 * height / 5;

    const size_t win_width = COLS;
    const int freqs_per_col = vis->f_results / win_width * 7/10;

    for (size_t i = 0; i < win_width; i++) {
        size_t bar_height = 0;
        for (int j = 0; j < freqs_per_col; j++)
            bar_height += vis->freq_magnitudes[i * freqs_per_col + j];
        bar_height = (bar_height / freqs_per_col < height)? bar_height / freqs_per_col: height;

        const size_t start_y = (yoff > 0)? yoff: height - bar_height;
        const size_t stop_y = (bar_height + start_y > LINES)? LINES: bar_height + start_y;
        for (size_t j = start_y; j < stop_y; j++)
            mvwprintw(vis->win.win, j, i, "|");
    }
}

static void (*draw_func) (struct visual_win *, int16_t *, ssize_t, size_t, size_t) = draw_wave;

static void update_viz(struct visual_win *vis)
{
    int16_t buf[SAMPLES];
    ssize_t data = read(vis->vis_fd, buf, sizeof(buf));
    if (data < 0)
        return ;

    ssize_t samples_read = data / sizeof(int16_t);

    werase(vis->win.win);
    if (SAMPLES == 2048) {
        (draw_func) (vis, buf, samples_read, 0, LINES);
    } else {
        int16_t buf_left[samples_read / 2], buf_right[samples_read / 2];
        for (ssize_t i = 0, j = 0; i < samples_read; i += 2, j++) {
            buf_left[j] = buf[i];
            buf_right[j] = buf[i + 1];
        }
        size_t half_height = LINES / 2;
        (draw_func) (vis, buf_left, samples_read / 2, 0, half_height);
        (draw_func) (vis, buf_right, samples_read / 2, half_height, half_height + 1);
    }
}

static void visual_win_update(struct nclyr_win *win)
{
    struct visual_win *vis = container_of(win, struct visual_win, win);
    update_viz(vis);
}

static void visual_win_init(struct nclyr_win *win)
{
    struct visual_win *vis = container_of(win, struct visual_win, win);
    vis->vis_fd = open("/tmp/mpd.fifo", O_RDONLY | O_NONBLOCK);

    vis->f_results = SAMPLES / 2 + 1;

    vis->freq_magnitudes = malloc(vis->f_results * sizeof(int));
    vis->f_input = malloc(sizeof(double) * SAMPLES);
    vis->f_output = malloc(sizeof(fftw_complex) * vis->f_results);
    vis->f_plan = fftw_plan_dft_r2c_1d(SAMPLES, vis->f_input, vis->f_output, FFTW_ESTIMATE);

    win->updated = 1;
}

static void visual_win_clear(struct nclyr_win *win)
{
    struct visual_win *vis = container_of(win, struct visual_win, win);

    close(vis->vis_fd);

    free(vis->f_output);
    free(vis->f_input);
    free(vis->freq_magnitudes);
}

static struct visual_win visual_window_init = {
    .win = {
        .win_name = "Visualizer",
        .win = NULL,
        .timeout = 50,
        .lyr_types = (const enum lyr_data_type[]) { LYR_DATA_TYPE_COUNT },
        .keypresses = (const struct nclyr_keypress[]) {
            N_END()
        },
        .init = visual_win_init,
        .clean = visual_win_clear,
        .switch_to = NULL,
        .update = visual_win_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = NULL,
    },
};

struct nclyr_win *visual_win_new(void)
{
    struct visual_win *win = malloc(sizeof(*win));
    memcpy(win, &visual_window_init, sizeof(visual_window_init));
    return &win->win;
}

