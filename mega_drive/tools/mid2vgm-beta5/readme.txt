MIDI-to-VGM translator  (beta 5)

file list:

m2v-dos.exe        16-bit DOS version
mid2vgm.exe        Win32 version
mid2vgm.elf        i386 Linux version
mid2vgm.x          X68000 version
mid2vgm.no         source for MIDI-to-VGM translator
opmfreq2.bin       data table used in the source code

xm2mid.exe         Win32 GUI XM-to-MIDI conversion tool
iscm.exe           Win32 GUI FM instrument test/edit tool
isc2dmp.*          for converting ISC instrument to DMP (deflemask)

opnz.*             16-bit DOS VGM player for ESFM, OPL3, my homemade soundcard, and maybe OPL2

opmplay.*          X68000 player for YM2151 VGMs

opll2mid.*         fun experiment in converting YM2413 VGMs to MIDI

isc\*.isc          instrument definitions used by mid2vgm


usage:

MID2VGM [options] file.mid [output.vgm]

  chip mode:
    -0 = 1x YM2151        8x 4-op FM channels
    -1 = 2x YM2151       16x 4-op FM channels
    -2 = 1x YM2612        6x 4-op FM channels, 3x square wave (optional), 1x noise (optional)
    -3 = 2x YM2612       12x 4-op FM channels, 3x square wave (optional), 1x noise (optional)
    -4 = 1x YM2608        6x 4-op FM channels, 6x rhythm sounds, 3x square wave (optional)
    -5 = 2x YM2608       12x 4-op FM channels, 6x rhythm sounds, 6x square wave (optional)
    -6 = 1x YM2203        3x 4-op FM channels, 3x square wave (optional)
    -7 = 2x YM2203        6x 4-op FM channels, 6x square wave (optional)
    -8 = 1x YMF262        9x 4-op FM channels
    -9 = 2x YMF262       18x 4-op FM channels
   -10 = 1x YM3812        4x 4-op FM channels
   -11 = 2x YM3812        8x 4-op FM channels
   -12 = ESFM            18x 4-op FM channels

Usage of the SN76489 square wave channels in YM2612 mode is significantly limited compared to
the square wave channels on YM2203/YM2608, because of the SN76489's narrower frequency range.
(mid2vgm won't attempt to play a track on PSG if it detects any notes that are out of range.)

On YM3812 and YMF262, two 2-op channels are paired together to make a 4-op channel. These chips
(as well as ESFM) don't have all the same algorithms as the OPM/OPN family, and so different
instrument sets are used (LGM*.ISC and LGP*.ISC).

  drum mode:
    -d1 = none               (default for chipmode 6,7,10)
    -d2 = use YM2608 rhythm  (default for chipmode 4,5 and only usable in these modes)
    -d3 = use FM drums       (default for chipmode 0,1,8,9,11,12)
    -d4 = use SN76 noise     (default for chipmode 2,3)

    -pX = attenuate drums - where X is in steps of .75dB
          (can be negative, to make percussion louder)

  square wave channel mode:  (only applies to chip modes 2,3,4,5,6,7)
    -s1 = allocate SSG channels for melody as needed
    -s2 = allocate all SSG channels for melody
    -aX = attenuate SSG volume - where X is in steps of .75dB

  other options:
    -tX = transpose - where X is number of semitones  (can be negative)
    -iX = instrument test - overrides all instruments in the MIDI input with the specified one
          (X is an instrument number)
    -one = use one-channel-one-sound allocation instead of LRKO
          (Likely to drop entire channels instead of individual notes when there is too much
           polyphony in a song. Also likely to make a smaller file.)
    -f1 = vibrato depth factor - 0=none, 2=default, 4=max  (-f4 works well with XM2MID)
    -r1 = reserve FM channel(s)  (useful when last channel should be open for DAC/sfx)

  relatively useless options:
    -l1 = limit notes per MIDI channel (where 1 is number of notes)
    -v1 = use linear volume table instead of MIDI std.
    -m1 = set minimum release time (in 44KHz ticks)

If no output filename specified, MOUT.VGM will be written.


changes since beta 4:

* added -r switch for reserved channels

* can specify output filename

* fixed SN76 drums using chan 3 freq  (which was never set to anything in particular)

* fixed header endianness on X68

* looks for ISC subdirectory in the directory where executable exists  (except in Linux)

* songs are now allowed to key-on a note when volume is zero
  (apparently this is a thing)

* XM2MID crash fix, support for EEx effect, and 'songloops' option

* OPNZ fixes

* added OPLL2MID to the archive
