#ifndef PS2_PAD_H
#define PS2_PAD_H

#ifdef PLAYSTATION2

#include <string.h>
#include <libpad.h>
#include <kernel.h>

// SDL joystick button indices used by the PS2 SDL port, in the same order the
// game's default joystick mapping expects. Using them keeps the game's
// button_codes configuration untouched while bypassing SDL's joystick layer,
// which hangs on PS2 (its init never returns with this SDK/emulator).
static const unsigned short ps2_pad_button_bits[12] = {
    PAD_SQUARE, PAD_CROSS, PAD_CIRCLE, PAD_TRIANGLE,
    PAD_SELECT, PAD_START, PAD_L1, PAD_R1,
    PAD_L2, PAD_R2, PAD_L3, PAD_R3
};

static char ps2_pad_buffer[256] __attribute__((aligned(64)));
static int ps2_pad_opened = 0;

static int ps2_pad_init(void)
{
    if (padInit(0) < 0) {
        return -1;
    }
    ps2_pad_opened = (padPortOpen(0, 0, ps2_pad_buffer) != 0);
    return ps2_pad_opened ? 0 : -1;
}

static int ps2_pad_available(void)
{
    return ps2_pad_opened;
}

/**
 * @brief Read port 0 buttons as a bitmask of PAD_* bits.
 *
 * Returns 0 while the pad is not ready, so callers see "nothing pressed"
 * instead of blocking (padGetState never blocks).
 */
static unsigned int ps2_pad_read_buttons(void)
{
    struct padButtonStatus status;
    int state;

    if (!ps2_pad_opened) {
        return 0;
    }

    state = padGetState(0, 0);
    if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1) {
        static int last_state = -1;
        if (state != last_state) {
            last_state = state;
        }
        return 0;
    }

    memset(&status, 0, sizeof(status));
    if (padRead(0, 0, &status) == 0) {
        return 0;
    }

    if (status.btns != 0xffff) {
    }

    // padButtonStatus.btns has 0 for pressed buttons.
    return (unsigned int)(0xffffu ^ status.btns);
}

/**
 * @brief Read the left analog stick.
 *
 * @param lx horizontal, -128 (left) .. 127 (right)
 * @param ly vertical,   -128 (up)   .. 127 (down)
 * @return 1 when a pad state was read, 0 otherwise.
 */
static int ps2_pad_read_axes(int *lx, int *ly)
{
    struct padButtonStatus status;
    int state;

    if (!ps2_pad_opened) {
        return 0;
    }

    state = padGetState(0, 0);
    if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1) {
        return 0;
    }

    memset(&status, 0, sizeof(status));
    if (padRead(0, 0, &status) == 0) {
        return 0;
    }

    // Pads in digital mode report 0,0 for the analog sticks.
    if (status.ljoy_h == 0 && status.ljoy_v == 0) {
        return 0;
    }

    *lx = (int)status.ljoy_h - 128;
    *ly = (int)status.ljoy_v - 128;
    return 1;
}

#endif // PLAYSTATION2

#endif // PS2_PAD_H
