// PS3 input test: lists SDL joysticks, logs every joystick/key event and the raw
// libio pad state to the TTY, and lights up one box per button on screen.
// Button order (SDL_PSL1GHT): 0 left, 1 down, 2 right, 3 up, 4 start, 5 R3, 6 L3,
// 7 select, 8 square, 9 cross, 10 circle, 11 triangle, 12 R1, 13 L1, 14 R2, 15 L2.
#include <SDL.h>
#include <io/pad.h>
#include <rsx/rsx.h>
#include <rsx/commands.h>
#include <cstdio>
#include <cstring>

// Same fix as the game (ports/ps3/ps3_platform.cpp): SDL_PSL1GHT's 64 KB RSX command
// buffer fills up after 256 flips and the program then hangs; wrap it in time.
extern "C" s32 rsxContextCallback(gcmContextData *context, u32 count);
static void rsx_guard()
{
    gcmContextData *ctx = gGcmContext;
    if (ctx && (u32)(ctx->end - ctx->current) < 2048) {
        rsxContextCallback(ctx, 4);
        rsxFlushBuffer(ctx);
    }
}

static void say(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#include <cstdarg>
static void say(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        say("PADTEST SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Surface *screen = SDL_SetVideoMode(640, 360, 32, SDL_SWSURFACE);
    if (!screen) {
        say("PADTEST SDL_SetVideoMode failed: %s", SDL_GetError());
        return 1;
    }

    const int n = SDL_NumJoysticks();
    say("PADTEST joysticks found: %d", n);
    for (int i = 0; i < n; i++) {
        say("PADTEST   joystick %d name=%s", i, SDL_JoystickName(i));
    }
    SDL_Joystick *joy = n > 0 ? SDL_JoystickOpen(0) : NULL;
    say("PADTEST SDL_JoystickOpen(0) = %p buttons=%d axes=%d", (void *)joy,
        joy ? SDL_JoystickNumButtons(joy) : -1, joy ? SDL_JoystickNumAxes(joy) : -1);
    SDL_JoystickEventState(SDL_ENABLE);

    bool pressed[16];
    memset(pressed, 0, sizeof(pressed));
    unsigned int last_raw = 0xFFFFFFFFu;
    int last_connected = -1;
    unsigned int frame = 0;
    bool running = true;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_JOYBUTTONDOWN:
                say("PADTEST joy button DOWN %d", ev.jbutton.button);
                if (ev.jbutton.button < 16) pressed[ev.jbutton.button] = true;
                break;
            case SDL_JOYBUTTONUP:
                say("PADTEST joy button UP   %d", ev.jbutton.button);
                if (ev.jbutton.button < 16) pressed[ev.jbutton.button] = false;
                break;
            case SDL_JOYAXISMOTION:
                say("PADTEST joy axis %d value %d", ev.jaxis.axis, ev.jaxis.value);
                break;
            case SDL_KEYDOWN:
                say("PADTEST key DOWN sym=%d", (int)ev.key.keysym.sym);
                break;
            case SDL_KEYUP:
                say("PADTEST key UP   sym=%d", (int)ev.key.keysym.sym);
                break;
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
            }
        }

        // Raw libio state, independent of SDL.
        padInfo info;
        if (ioPadGetInfo(&info) == 0) {
            if ((int)info.connected != last_connected) {
                last_connected = (int)info.connected;
                say("PADTEST libio: connected=%d status0=%d", (int)info.connected, (int)info.status[0]);
            }
            padData d;
            memset(&d, 0, sizeof(d));
            if (ioPadGetData(0, &d) == 0 && d.len > 0) {
                // Boxes follow the decoded libio state (SDL joystick events are not
                // used: SDL_PSL1GHT counts pads only at startup and may see none).
                pressed[0] = d.BTN_LEFT;  pressed[1] = d.BTN_DOWN;   pressed[2] = d.BTN_RIGHT;
                pressed[3] = d.BTN_UP;    pressed[4] = d.BTN_START;  pressed[5] = d.BTN_R3;
                pressed[6] = d.BTN_L3;    pressed[7] = d.BTN_SELECT; pressed[8] = d.BTN_SQUARE;
                pressed[9] = d.BTN_CROSS; pressed[10] = d.BTN_CIRCLE; pressed[11] = d.BTN_TRIANGLE;
                pressed[12] = d.BTN_R1;   pressed[13] = d.BTN_L1;    pressed[14] = d.BTN_R2;
                pressed[15] = d.BTN_L2;
                // Every report that arrives: first 16 raw bytes + decoded buttons.
                const unsigned char *raw = (const unsigned char *)&d;
                say("PADTEST report len=%d raw=%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x  L%d D%d R%d U%d st%d se%d X%d sq%d O%d tri%d L1=%d R1=%d L2=%d R2=%d anaL=%d,%d",
                    (int)d.len, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7],
                    raw[8], raw[9], raw[10], raw[11], raw[12], raw[13], raw[14], raw[15],
                    (int)d.BTN_LEFT, (int)d.BTN_DOWN, (int)d.BTN_RIGHT, (int)d.BTN_UP,
                    (int)d.BTN_START, (int)d.BTN_SELECT, (int)d.BTN_CROSS, (int)d.BTN_SQUARE,
                    (int)d.BTN_CIRCLE, (int)d.BTN_TRIANGLE, (int)d.BTN_L1, (int)d.BTN_R1,
                    (int)d.BTN_L2, (int)d.BTN_R2, (int)d.ANA_L_H, (int)d.ANA_L_V);
            }
        }

        // Draw: top bar = joystick count (one green block each), then 4x4 grid of buttons.
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 20, 20, 40));
        for (int i = 0; i < n && i < 8; i++) {
            SDL_Rect r = { (Sint16)(20 + i * 40), 10, 30, 20 };
            SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 200, 0));
        }
        if (n == 0) {
            SDL_Rect r = { 20, 10, 200, 20 };
            SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 200, 0, 0)); // red: no joystick
        }
        for (int b = 0; b < 16; b++) {
            SDL_Rect r = { (Sint16)(20 + (b % 4) * 100), (Sint16)(60 + (b / 4) * 70), 90, 60 };
            SDL_FillRect(screen, &r, pressed[b] ? SDL_MapRGB(screen->format, 255, 220, 0)
                                                : SDL_MapRGB(screen->format, 60, 60, 90));
        }
        rsx_guard();
        SDL_Flip(screen);
        SDL_Delay(16);
        frame++;
        if (frame == 1) { say("PADTEST running (frame loop started)"); }
    }
    SDL_Quit();
    return 0;
}
