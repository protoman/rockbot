# Rockbot 2.23.000 release notes (since 18 July)

## New platforms
- Nintendo Switch (devkitA64, NRO package) and PlayStation Vita (VitaSDK, VPK package).
- PlayStation 3 (PSL1GHT, pkg with XMB icon and background per game). Tested on real hardware: saves, level finish and PS-button handling work.
- GameCube, built as a Wii variant.
- Sega Mega Drive port, with:
  - real room transitions, parallax, and the classic HUD and menus;
  - a stage editor and enemy AI like the PC;
  - boss intros, charged shots, shields and slide/dash trails;
  - the ending parade and credits scroll, and an audit of its differences from the PC version.

## Existing ports
- PS2, PSP, Dingux and PocketGo now build with CMake and the latest SDKs.
- PSP:
  - pixel-perfect integer scaling, with borders;
  - libxmp-lite for the tracker music, replacing libmodplug, which crashed on hardware;
  - Rockbot 2 icon and background;
  - the default graphics mode is LOW;
  - many fixes found on real hardware: file paths, the VFPU attribute, a division by zero, and the heap raised to 28 MB.
- Android:
  - much smaller APKs (native libraries kept compressed, 169 MB to 61 MB);
  - fixed tracker (.mod) music noise on 64-bit devices, so the MP3 conversion is gone;
  - the APK name includes the game number.
- The editor builds with CMake on both Qt5 and Qt6.
- Windows zip names now say Win64 or Win32.

## Gameplay fixes
- Energy and special tanks now fill the player's HP; before, they added to the current and maximum HP.
- A player shot against an enemy that is immune to that weapon is now used up instead of flying through.
- Shoryuken can only start on the ground; before, it could be chained in the air.
- Ladder and ground detection is fixed, and belts and ice are solid from the sides and below.
- AI reactions whose action is WALK were being ignored; they now run.
- A boss intro with no boss nearby no longer freezes every enemy's AI.
- Zigzag projectiles bounce only off walls.
- Objects settle on the ground while the map loads, and teleporting NPCs no longer land inside walls.
- Fixed an out-of-bounds read when a tank or item is the selected weapon.
- The ending scenes can't be skipped with Start.
- The classic stage-select cursor pulses twice as fast.

## Graphics and files
- The rotated missile and other zoomed sprites on 16-bit screens (PS3, PSP, Dingux, PS2 and similar) were opaque. Fixed with one shared helper (not yet seen on a device).
- Every file open, read, write and close now reports why it failed, and retries.
- Reads are no longer opened in append mode.
- The game exits cleanly when a required file can't be loaded.
- On the PS3, created files are made world-writable (0666) so saves can be overwritten.

## Performance and tooling
- Faster scroll locks, animated tiles, enemy reactions and parked enemies. The in-game menu opens quickly on handhelds.
- `release_all.sh` can build every platform in Docker and tags the release in git. New Docker guide and rewritten README.
