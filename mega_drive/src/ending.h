#ifndef ENDING_H
#define ENDING_H

#include <genesis.h>

// PC game::show_ending after the final castle: player walking parade
// (three parallax layers), boss parade and the game credits.
void ending_run(bool new_characters);   // new_characters: first completion, show the unlock notice

// Credits-only screen, used by the title ABOUT/EXTRAS menu (PC show_credits).
void ending_credits_run(void);

#endif
