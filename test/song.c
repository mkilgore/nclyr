
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "song.h"
#include "test/test.h"

struct song_fake {
    struct song_info song;
    int fake_data;
};

static void song_fake_free(struct song_info *song)
{
    song_info_clear(song);
    free(song);
}

void song_fake_init(struct song_fake *fake);

static struct song_info *song_fake_copy(const struct song_info *song)
{
    struct song_fake *fake = container_of(song, struct song_fake, song);
    struct song_fake *new = malloc(sizeof(*new));

    song_fake_init(new);
    song_info_copy(&fake->song, &new->song);
    new->fake_data = fake->fake_data;
    return &new->song;
}

void song_fake_init(struct song_fake *fake)
{
    song_init(&fake->song);
    fake->song.sfree = song_fake_free;
    fake->song.copy = song_fake_copy;
    fake->fake_data = 0;
}

int song_copy_test(void)
{
    int ret = 0;
    struct song_fake *fake = malloc(sizeof(*fake));
    struct song_info *info;
    struct song_fake *new;

    song_fake_init(fake);

    fake->fake_data = 2;

    info = song_copy(&fake->song);
    new = container_of(info, struct song_fake, song);

    /* Test that copy callback works */
    ret += test_assert(fake->fake_data == new->fake_data);

    song_free(&fake->song);
    song_free(&new->song);

    return ret;
}

#define ARTIST "Artist1"
#define TITLE  "Title1"
#define ALBUM  "Album1"

int song_equal_test(void)
{
    int ret = 0;
    struct song_info song1, song2;

    song_init(&song1);
    song_init(&song2);

    song1.tag.artist = strdup(ARTIST);
    song1.tag.album = strdup(ALBUM);
    song1.tag.title = strdup(TITLE);

    song2.tag.artist = strdup(ARTIST);
    song2.tag.album = strdup(ALBUM);
    song2.tag.title = strdup(TITLE);

    ret += test_assert(song_equal(&song1, &song2));

    free(song1.tag.artist);
    song1.tag.artist = NULL;

    ret += test_assert(!song_equal(&song1, &song2));

    song1.tag.artist = strdup(ARTIST);
    free(song1.tag.album);
    song1.tag.album = NULL;

    ret += test_assert(!song_equal(&song1, &song2));

    song1.tag.album = strdup(ALBUM);
    free(song1.tag.title);
    song1.tag.title = NULL;

    ret += test_assert(!song_equal(&song1, &song2));

    song_info_clear(&song1);
    song_info_clear(&song2);

    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {
        { song_equal_test, "Song test" },
        { song_copy_test, "Song copy" },
    };

    ret = run_tests("Test Song system", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}


