#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <poll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>


int init_xcb(xcb_connection_t** conn, xcb_randr_crtc_t** crtcs, int* numCrtcs);
void set_gamma(xcb_connection_t* conn, xcb_randr_crtc_t crtc, int size, double r, double g, double b);

int main() {
    // signal handler
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        return 1;
    }

    int sfd = signalfd(-1, &mask, 0);
    if (sfd < 0) {
        return 1;
    }

    struct pollfd pfd = { .fd = sfd, .events = POLLIN };

    xcb_connection_t* conn;
    xcb_randr_crtc_t* allCrtcs;
    int numCrtcs;

    if (init_xcb(&conn, &allCrtcs, &numCrtcs) < 0) {
        return 1;
    }

    xcb_randr_get_crtc_gamma_size_cookie_t* gammaSizeCookies = malloc(numCrtcs * sizeof(xcb_randr_get_crtc_gamma_size_cookie_t));

    for (int i = 0; i < numCrtcs; i++) {
        gammaSizeCookies[i] = xcb_randr_get_crtc_gamma_size(conn, allCrtcs[i]);
    }

    xcb_randr_get_crtc_gamma_size_reply_t** gammaSizeReplies = malloc(numCrtcs * sizeof(void*));

    for (int i = 0; i < numCrtcs; i++) {
        gammaSizeReplies[i] = xcb_randr_get_crtc_gamma_size_reply(conn, gammaSizeCookies[i], NULL);
    }

    free(gammaSizeCookies);

    struct signalfd_siginfo si;

    // main loop
    for (int t = 0; ; t++) {
        double red = fmax(cos(t / M_PI), 0.0);
        double green = fmax(cos(t / M_PI + 2), 0.0);
        double blue = fmax(cos(t / M_PI + 4), 0.0);

        poll(&pfd, 1, 0);
        int hasSignal = pfd.revents & POLLIN;

        for (int i = 0; i < numCrtcs; i++) {
            int len = gammaSizeReplies[i]->size;

            if (hasSignal) {
                set_gamma(conn, allCrtcs[i], len, 1.0, 1.0, 1.0);
            } else {
                set_gamma(conn, allCrtcs[i], len, red, green, blue);
            }
        }

        usleep(40000);

        if (hasSignal) break;
    }

    free(gammaSizeReplies);
    free(allCrtcs);
    xcb_disconnect(conn);
    close(sfd);

    return 0;
}

int init_xcb(xcb_connection_t** conn, xcb_randr_crtc_t** crtcs, int* numCrtcs) {
    *conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(*conn)) {
        return -1;
    }

    const xcb_setup_t* setup = xcb_get_setup(*conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    int numScreens = xcb_setup_roots_length(setup);

    xcb_randr_get_screen_resources_cookie_t* screenCookies = malloc(numScreens * sizeof(xcb_randr_get_screen_resources_cookie_t));

    for (int i = 0; i < numScreens; i++) {
        xcb_screen_t* screen = iter.data;
        screenCookies[i] = xcb_randr_get_screen_resources(*conn, screen->root);
        xcb_screen_next(&iter);
    }

    xcb_randr_get_screen_resources_reply_t** screenReplies = malloc(numScreens * sizeof(void*));

    for (int i = 0; i < numScreens; i++) {
        screenReplies[i] = xcb_randr_get_screen_resources_reply(*conn, screenCookies[i], NULL);
    }

    free(screenCookies);

    xcb_randr_crtc_t** screenCrtcs = malloc(numScreens * sizeof(void*));
    int* crtcLengths = malloc(numScreens * sizeof(int));

    for (int i = 0; i < numScreens; i++) {
        screenCrtcs[i] = xcb_randr_get_screen_resources_crtcs(screenReplies[i]);
        crtcLengths[i] = xcb_randr_get_screen_resources_crtcs_length(screenReplies[i]);
    }

    free(screenReplies);

    *numCrtcs = 0;
    for (int i = 0; i < numScreens; i++) {
        *numCrtcs += crtcLengths[i];
    }

    *crtcs = malloc(*numCrtcs * sizeof(xcb_randr_crtc_t));

    int count = 0;
    for (int i = 0; i < numScreens; i++) {
        // memcpy?
        for (int j = 0; j < crtcLengths[i]; j++) {
            (*crtcs)[count++] = screenCrtcs[i][j];
        }
    }

    free(screenCrtcs);
    free(crtcLengths);

    return 0;
}

// f(x) = x^{\frac{1}{\gamma}}
// x \in [0, 1], must be scaled to and from uint16_t
void set_gamma(xcb_connection_t* conn, xcb_randr_crtc_t crtc, int size, double r, double g, double b) {
    uint16_t* red = malloc(size * sizeof(uint16_t));
    uint16_t* green = malloc(size * sizeof(uint16_t));
    uint16_t* blue = malloc(size * sizeof(uint16_t));

    double expR = 1 / r;
    double expG = 1 / g;
    double expB = 1 / b;
    
    for (int i = 0; i < size; i++) {
        double scaledX = (double) i / (double) (size - 1);
        
        red[i] = pow(scaledX, expR) * UINT16_MAX;
        green[i] = pow(scaledX, expG) * UINT16_MAX;
        blue[i] = pow(scaledX, expB) * UINT16_MAX;
    }

    xcb_void_cookie_t gammaCookie = xcb_randr_set_crtc_gamma(conn, crtc, size, red, green, blue);
    if (xcb_request_check(conn, gammaCookie)) {
        fprintf(stderr, "failed to change gamma\n");
    }

    free(red);
    free(green);
    free(blue);
}