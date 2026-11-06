#include "fm_patch.h"

namespace mod2vgm {

FmPatch FmPatchLibrary::get_distortion_guitar() {
	// Classic Mega Drive crunchy distortion guitar (Algorithm 4 / 5, high feedback on Op 1/3)
	FmPatch p;
	p.name = "DistGuitar";
	p.algorithm = 4;
	p.feedback = 6; // Heavy feedback on modulator produces harsh rich harmonics

	// Op 1 (Modulator 1)
	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 12; p.op[0].ar = 31; p.op[0].dr = 10; p.op[0].sr = 4; p.op[0].rr = 10; p.op[0].sl = 2;
	// Op 2 (Modulator 2)
	p.op[1].dt = 3; p.op[1].mul = 2; p.op[1].tl = 16; p.op[1].ar = 31; p.op[1].dr = 12; p.op[1].sr = 2; p.op[1].rr = 10; p.op[1].sl = 3;
	// Op 3 (Modulator 3 / detuned)
	p.op[2].dt = 7; p.op[2].mul = 1; p.op[2].tl = 20; p.op[2].ar = 31; p.op[2].dr = 14; p.op[2].sr = 0; p.op[2].rr = 9;  p.op[2].sl = 4;
	// Op 4 (Carrier)
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 6;  p.op[3].sr = 2; p.op[3].rr = 10; p.op[3].sl = 1;

	return p;
}

FmPatch FmPatchLibrary::get_clean_guitar() {
	FmPatch p;
	p.name = "CleanGuitar";
	p.algorithm = 2;
	p.feedback = 4;

	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 20; p.op[0].ar = 31; p.op[0].dr = 16; p.op[0].sr = 4; p.op[0].rr = 8; p.op[0].sl = 4;
	p.op[1].dt = 0; p.op[1].mul = 3; p.op[1].tl = 24; p.op[1].ar = 31; p.op[1].dr = 14; p.op[1].sr = 2; p.op[1].rr = 8; p.op[1].sl = 4;
	p.op[2].dt = 2; p.op[2].mul = 1; p.op[2].tl = 16; p.op[2].ar = 31; p.op[2].dr = 12; p.op[2].sr = 2; p.op[2].rr = 8; p.op[2].sl = 3;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 10; p.op[3].sr = 2; p.op[3].rr = 8; p.op[3].sl = 2;

	return p;
}

FmPatch FmPatchLibrary::get_slap_bass() {
	FmPatch p;
	p.name = "SlapBass";
	p.algorithm = 2;
	p.feedback = 7;

	// In rock bass: snappy attack, medium decay, and natural release decay (SR 6..10) to prevent droning
	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 10; p.op[0].ar = 31; p.op[0].dr = 18; p.op[0].sr = 12; p.op[0].rr = 14; p.op[0].sl = 6;
	p.op[1].dt = 0; p.op[1].mul = 2; p.op[1].tl = 18; p.op[1].ar = 31; p.op[1].dr = 16; p.op[1].sr = 10; p.op[1].rr = 12; p.op[1].sl = 7;
	p.op[2].dt = 1; p.op[2].mul = 1; p.op[2].tl = 22; p.op[2].ar = 31; p.op[2].dr = 14; p.op[2].sr = 8;  p.op[2].rr = 10; p.op[2].sl = 8;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 10; p.op[3].sr = 8;  p.op[3].rr = 14; p.op[3].sl = 3;

	return p;
}

FmPatch FmPatchLibrary::get_snare() {
	FmPatch p;
	p.name = "Snare";
	p.algorithm = 0;
	p.feedback = 7;

	p.op[0].dt = 0; p.op[0].mul = 8; p.op[0].tl = 8;  p.op[0].ar = 31; p.op[0].dr = 28; p.op[0].sr = 0; p.op[0].rr = 15; p.op[0].sl = 10;
	p.op[1].dt = 0; p.op[1].mul = 3; p.op[1].tl = 12; p.op[1].ar = 31; p.op[1].dr = 25; p.op[1].sr = 0; p.op[1].rr = 15; p.op[1].sl = 10;
	p.op[2].dt = 0; p.op[2].mul = 6; p.op[2].tl = 10; p.op[2].ar = 31; p.op[2].dr = 26; p.op[2].sr = 0; p.op[2].rr = 15; p.op[2].sl = 10;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 22; p.op[3].sr = 0; p.op[3].rr = 15; p.op[3].sl = 10;

	return p;
}

FmPatch FmPatchLibrary::get_hihat() {
	FmPatch p;
	p.name = "HiHat";
	p.algorithm = 0;
	p.feedback = 7;

	p.op[0].dt = 0; p.op[0].mul = 15; p.op[0].tl = 14; p.op[0].ar = 31; p.op[0].dr = 31; p.op[0].sr = 0; p.op[0].rr = 15; p.op[0].sl = 15;
	p.op[1].dt = 0; p.op[1].mul = 14; p.op[1].tl = 16; p.op[1].ar = 31; p.op[1].dr = 31; p.op[1].sr = 0; p.op[1].rr = 15; p.op[1].sl = 15;
	p.op[2].dt = 0; p.op[2].mul = 12; p.op[2].tl = 18; p.op[2].ar = 31; p.op[2].dr = 31; p.op[2].sr = 0; p.op[2].rr = 15; p.op[2].sl = 15;
	p.op[3].dt = 0; p.op[3].mul = 10; p.op[3].tl = 2;  p.op[3].ar = 31; p.op[3].dr = 31; p.op[3].sr = 0; p.op[3].rr = 15; p.op[3].sl = 15;

	return p;
}

FmPatch FmPatchLibrary::get_default_lead() {
	FmPatch p;
	p.name = "Lead";
	p.algorithm = 4;
	p.feedback = 5;

	// Op 1 (modulator)
	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 20; p.op[0].ar = 31; p.op[0].dr = 12; p.op[0].sr = 2; p.op[0].rr = 8; p.op[0].sl = 2;
	// Op 2
	p.op[1].dt = 0; p.op[1].mul = 2; p.op[1].tl = 25; p.op[1].ar = 31; p.op[1].dr = 10; p.op[1].sr = 0; p.op[1].rr = 7; p.op[1].sl = 3;
	// Op 3
	p.op[2].dt = 0; p.op[2].mul = 1; p.op[2].tl = 30; p.op[2].ar = 31; p.op[2].dr = 8;  p.op[2].sr = 0; p.op[2].rr = 6; p.op[2].sl = 4;
	// Op 4 (carrier)
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 6;  p.op[3].sr = 0; p.op[3].rr = 8; p.op[3].sl = 0;

	return p;
}

FmPatch FmPatchLibrary::get_default_bass() {
	FmPatch p;
	p.name = "SynthBass";
	p.algorithm = 2;
	p.feedback = 6;

	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 15; p.op[0].ar = 31; p.op[0].dr = 15; p.op[0].sr = 0; p.op[0].rr = 12; p.op[0].sl = 3;
	p.op[1].dt = 0; p.op[1].mul = 1; p.op[1].tl = 22; p.op[1].ar = 31; p.op[1].dr = 12; p.op[1].sr = 0; p.op[1].rr = 10; p.op[1].sl = 4;
	p.op[2].dt = 0; p.op[2].mul = 2; p.op[2].tl = 28; p.op[2].ar = 31; p.op[2].dr = 10; p.op[2].sr = 0; p.op[2].rr = 8;  p.op[2].sl = 5;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 8;  p.op[3].sr = 0; p.op[3].rr = 14; p.op[3].sl = 1;

	return p;
}

FmPatch FmPatchLibrary::get_default_pad() {
	FmPatch p;
	p.name = "SoftPad";
	p.algorithm = 5;
	p.feedback = 3;

	p.op[0].dt = 0; p.op[0].mul = 1; p.op[0].tl = 24; p.op[0].ar = 20; p.op[0].dr = 10; p.op[0].sr = 2; p.op[0].rr = 6; p.op[0].sl = 2;
	p.op[1].dt = 0; p.op[1].mul = 1; p.op[1].tl = 18; p.op[1].ar = 18; p.op[1].dr = 10; p.op[1].sr = 2; p.op[1].rr = 6; p.op[1].sl = 2;
	p.op[2].dt = 0; p.op[2].mul = 2; p.op[2].tl = 32; p.op[2].ar = 16; p.op[2].dr = 8;  p.op[2].sr = 2; p.op[2].rr = 6; p.op[2].sl = 2;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 2;  p.op[3].ar = 18; p.op[3].dr = 6;  p.op[3].sr = 1; p.op[3].rr = 6; p.op[3].sl = 0;

	return p;
}

FmPatch FmPatchLibrary::get_default_percussion() {
	FmPatch p;
	p.name = "Percussion";
	p.algorithm = 0;
	p.feedback = 7;

	p.op[0].dt = 0; p.op[0].mul = 4; p.op[0].tl = 10; p.op[0].ar = 31; p.op[0].dr = 25; p.op[0].sr = 0; p.op[0].rr = 15; p.op[0].sl = 8;
	p.op[1].dt = 0; p.op[1].mul = 1; p.op[1].tl = 12; p.op[1].ar = 31; p.op[1].dr = 22; p.op[1].sr = 0; p.op[1].rr = 15; p.op[1].sl = 8;
	p.op[2].dt = 0; p.op[2].mul = 2; p.op[2].tl = 15; p.op[2].ar = 31; p.op[2].dr = 20; p.op[2].sr = 0; p.op[2].rr = 15; p.op[2].sl = 8;
	p.op[3].dt = 0; p.op[3].mul = 1; p.op[3].tl = 0;  p.op[3].ar = 31; p.op[3].dr = 18; p.op[3].sr = 0; p.op[3].rr = 15; p.op[3].sl = 8;

	return p;
}

} // namespace mod2vgm
