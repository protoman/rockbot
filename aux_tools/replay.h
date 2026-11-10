#ifndef REPLAY_H
#define REPLAY_H

/**
 * Deterministic replay for tests (PC builds only, off unless the environment asks for it).
 *
 * ROCKBOT_REPLAY=<script>   run with a virtual clock (no sleeping, time only moves when the game "waits") and feed
 *                           the buttons from the script
 * ROCKBOT_TRACE=<file>      write one line per game frame with the state of the player and the enemies
 *
 * Script: one line per change, "<ms> <token> [<token> ...]". Tokens: +NAME press, -NAME release (NAME is UP DOWN LEFT
 * RIGHT JUMP ATTACK DASH SHIELD L R START), "END" finishes the run (flushes the trace and exits). '#' starts a comment.
 *
 * Same script + same build = same trace, so a refactoring can be checked by comparing traces before and after.
 * Run with HOME pointing to an empty folder (saves and config are written there) and SDL_VIDEODRIVER=dummy
 * SDL_AUDIODRIVER=dummy.
 */

#ifdef PC

namespace replay {
    bool enabled();                      // true when ROCKBOT_REPLAY is set
    unsigned long now();                 // virtual time in ms
    void advance(int ms);                // the game "slept"
    void update_buttons();               // applies the script events due at the current virtual time
    int held(int button);                // -1: the script never touched it, 0: released, 1: pressed
    bool tracing();
    void trace(const char *format, ...); // one line in the trace file
}

#else

namespace replay {
    inline bool enabled() { return false; }
    inline unsigned long now() { return 0; }
    inline void advance(int) {}
    inline void update_buttons() {}
    inline int held(int) { return -1; }
    inline bool tracing() { return false; }
    inline void trace(const char *, ...) {}
}

#endif

#endif // REPLAY_H
