
/*
    Particle is a super simple screen magnifier for x11
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>


bool running = true;

int screen;
int dpy_width;
int dpy_height;
Visual *visual;
int depth;
Window win;
Window root;
Display *dpy;
GC gc;

int x = 0;
int y = 0;

#define padding 2
#define box 512
#define wbs (box + (padding * 2))

uint8_t ratio = 4;
int32_t capture = 128;
int32_t half_capture = 64;

uint8_t image_data[box * box * 4];

void do_image(void) {
    int lx = x;
    int ly = y;

    if (lx > half_capture) lx -= half_capture;
    else lx = 0;

    if (ly > half_capture) ly -= half_capture;
    else ly = 0;

    lx = lx + capture >= dpy_width ? dpy_width - capture : lx;
    ly = ly + capture >= dpy_height ? dpy_height - capture : ly;

    XImage *img = XGetImage(
        dpy, root,
        lx, ly, capture, capture,
        AllPlanes, ZPixmap
    );

    for (int sy = 0; sy < capture; sy++) {
        for (int sx = 0; sx < capture; sx++) {
            uint32_t i = (sy * capture + sx) * 4;

            uint32_t base_px = (sy * box * ratio) + (sx * ratio);
            for (uint8_t ry = 0; ry < ratio; ry++) {
                uint32_t bn = (base_px + (ry * box)) * 4;
                for (uint8_t rx = 0; rx < ratio; rx++) {
                    image_data[bn + 0] = img->data[i + 0];
                    image_data[bn + 1] = img->data[i + 1];
                    image_data[bn + 2] = img->data[i + 2];
                    // image_data[j + 3] = starlight.out.s[i + 3];
                    bn += 4;
                }
            }
        }
    }

    XImage *img_2 = XCreateImage(
        dpy, visual, depth, ZPixmap, 0,
        (char *)image_data, box, box, 32, 0
    );

    XPutImage(dpy, win, gc, img_2, 0, 0, padding, padding, box, box);

    XDestroyImage(img);
    // XDestroyImage(img_2);
}

int main(void) {

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        printf("con't open display :(");
        return -1;
    }

    screen = DefaultScreen(dpy);
    dpy_width = DisplayWidth(dpy, screen);
    dpy_height = DisplayHeight(dpy, screen);
    visual = DefaultVisual(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    root = DefaultRootWindow(dpy);

    win = XCreateSimpleWindow(
        dpy, root,
        dpy_width - (wbs + 10), dpy_height - (wbs + 10),
        box + padding, box + padding,
        0, 0, 0xDB6A0B
    );

    gc = XCreateGC(dpy, win, 0, NULL);

    XSizeHints sh;
    sh.min_width  = sh.max_width  = wbs;
    sh.min_height = sh.max_height = wbs;
    sh.flags = PMinSize | PMaxSize;
    XSetWMNormalHints(dpy, win, &sh);

    XStoreName(dpy, win, "Particle");
    XMapWindow(dpy, win);
    XSelectInput(dpy, win, KeyPressMask | ExposureMask);
    XFlush(dpy);


    Window g;
    int i;
    uint32_t m = 0;

    XQueryPointer(
        dpy, root, &g, &g,
        &x, &y, &i, &i, &m
    );

    Cursor c = XCreateFontCursor(dpy, 2);

    XGrabPointer(
        dpy, root, true,
        PointerMotionMask | ButtonPressMask,
        GrabModeAsync, GrabModeAsync, None,
        c, CurrentTime
    );

    uint32_t n = 0;
    XEvent ev;
    XSync(dpy, false);
    while (running) {
        bool found = XCheckMaskEvent(
            dpy,
            (KeyPressMask | ExposureMask | 
             PointerMotionMask | ButtonPressMask),
            &ev
        );

        if (n > 20000) {
            n = 0;
            do_image();
        }

        if (!found) {
            n++;
            continue;
        }

        if (ev.type == KeyPress || ev.type == KeyRelease) {
            switch (ev.xkey.keycode) {
                case 9:  // ESC
                case 24: // Q
                    running = false;
                    break;

                case 40:  // D
                case 114: // Arrow Right
                    x++;
                    do_image();
                    break;

                case 38:  // A
                case 113: // Arrow left
                    x--;
                    do_image();
                    break;

                case 25:  // W
                case 111: // Arrow Up
                    y--;
                    do_image();
                    break;

                case 39:  // S
                case 116: // Down
                    y++;
                    do_image();
                    break;
            }
        } else if (ev.type == Expose) {
            do_image();
        } else if (ev.type == MotionNotify) {
            x = ev.xmotion.x_root;
            y = ev.xmotion.y_root;
            do_image();
        } else if (ev.type == ButtonPress) {
            x = ev.xmotion.x_root;
            y = ev.xmotion.y_root;

            if (ev.xbutton.button == 4) {
                if (capture <= 4) continue;
                ratio *= 2;
                capture = box / ratio;
                half_capture = capture / 2;
                do_image();
            } else if (ev.xbutton.button == 5) {
                if (ratio < 2) continue;
                ratio /= 2;
                capture = box / ratio;
                half_capture = capture / 2;
                do_image();
            } else if (ev.xbutton.button == 3) {
                int32_t mx = ev.xbutton.x_root - box / 2;
                int32_t my = ev.xbutton.y_root - box / 2;
                if (mx < 10) mx = 10;
                if (my < 10) my = 10;
                if (mx > dpy_width - box - 10) mx = dpy_width - box - 10;
                if (my > dpy_height - box - 10) my = dpy_height - box - 10;

                XMoveWindow(dpy, win, mx, my);
            } else {
                running = false;
            }
        }
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}

