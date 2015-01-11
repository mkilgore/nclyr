
#include "common.h"

#include "player.h"
#include "pianobar/pianobar.h"

struct player players[] = {
    { "pianobar", pianobar_setup_notification, pianobar_stop_notification },
    { NULL }
};

struct player *player_current_used(void)
{
    return players;
}

void player_setup_notification(int pipefd)
{
    pianobar_setup_notification(pipefd);
    return ;
}

void player_stop_notification(void)
{
    pianobar_stop_notification();
    return ;
}

void player_notification_free(struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_NO_SONG:

        break;
    case PLAYER_SONG:
        song_clear(&notif->u.song);
        break;
    }
}

