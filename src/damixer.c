
/*
    gcc -Os -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -lasound
    damixer.c -o damixer
*/

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <alsa/asoundlib.h>

#define DAMIXER_CHANNEL "damixer.channel"

void run(void);
void keypress(XEvent* e);
void update_status(void);

/* variables */
int channel = -1;
Display* dpy;
Window root;

/* ALSA mute/deafen hotkeys */
snd_ctl_t* ALSA_control;
snd_ctl_elem_value_t* ALSA_capture_d;
snd_ctl_elem_value_t* ALSA_playback_d;


void run(void) {
    XEvent ev;
    XSync(dpy, true);

    while (!XNextEvent(dpy, &ev)) {
        if (ev.type != KeyPress) continue;
        keypress(&ev);
    }
}

void keypress(XEvent* e) {
    bool is_deafen;

    // snd_ctl_elem_read(ALSA_control, ALSA_capture_d);
    snd_ctl_elem_read(ALSA_control, ALSA_playback_d);

    is_deafen = !snd_ctl_elem_value_get_boolean(ALSA_playback_d, 0);


    if (e->xkey.keycode == 72) {
        // F6 - mute
        snd_ctl_elem_value_set_boolean(ALSA_capture_d, 0, false);
    } else if (e->xkey.keycode == 73) {
        // F7 - unmute
        snd_ctl_elem_value_set_boolean(ALSA_capture_d, 0, true);

        if (is_deafen) {
            // undeafen
            snd_ctl_elem_value_set_boolean(ALSA_playback_d, 0, true);
        }
    } else if (e->xkey.keycode == 74) {
        // F8

        if (is_deafen) {
            // undeafen
            snd_ctl_elem_value_set_boolean(ALSA_playback_d, 0, true);
        } else {
            // mute and deafen
            snd_ctl_elem_value_set_boolean(ALSA_capture_d, 0, false);
            snd_ctl_elem_value_set_boolean(ALSA_playback_d, 0, false);
        }
    }

    snd_ctl_elem_write(ALSA_control, ALSA_capture_d);
    snd_ctl_elem_write(ALSA_control, ALSA_playback_d);

    update_status();
}


void update_status(void) {
    bool status[2];

    snd_ctl_elem_read(ALSA_control, ALSA_capture_d);
    snd_ctl_elem_read(ALSA_control, ALSA_playback_d);

    status[0] = snd_ctl_elem_value_get_boolean(ALSA_capture_d, 0);
    status[1] = snd_ctl_elem_value_get_boolean(ALSA_playback_d, 0);

    lseek(channel, 0, SEEK_SET);
    write(channel, status, sizeof(status));
}


int main(void) {
    char path[512];
    char username[32];

    if (getlogin_r(username, sizeof(username))) {
        printf("error getting the username\n");
        return -1;
    }

    snprintf(path, sizeof(path), "/tmp/%s."DAMIXER_CHANNEL, username);

    channel = open(path, O_SYNC | O_WRONLY | O_CREAT | O_TRUNC, 00666);
    if (channel == -1) {
        printf("error opening the channel\n");
        return 1;
    }

    if (!(dpy = XOpenDisplay(NULL)))
        return 1;

    root = DefaultRootWindow(dpy);

    XGrabKey(dpy, 72, 0, root, true, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, 73, 0, root, true, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, 74, 0, root, true, GrabModeAsync, GrabModeAsync);

    /* init ALSA stuff */
    snd_ctl_elem_value_alloca(&ALSA_capture_d);
    snd_ctl_elem_value_alloca(&ALSA_playback_d);

    snd_ctl_elem_value_set_numid(ALSA_capture_d, 2);
    snd_ctl_elem_value_set_numid(ALSA_playback_d, 4);

    if (snd_ctl_open(&ALSA_control, "default", 0)) {
        printf("ALSA not opening.\n");
        XCloseDisplay(dpy);
        return 1;
    }

    update_status();
    run();

    snd_ctl_close(ALSA_control);
    XCloseDisplay(dpy);
    close(channel);

    return 0;
}
