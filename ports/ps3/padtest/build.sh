#!/usr/bin/env bash
# Builds the tiny PS3 input test (padtest.cpp) into build-ps3/padtest/PS3_GAME.
# Run it with: rpcs3 build-ps3/padtest/PS3_GAME/USRDIR/EBOOT.BIN
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
PS3DEV="${PS3DEV:-/mnt/ExtraData/Devel/SDK/PS3/ps3dev}"
export PS3DEV PSL1GHT="$PS3DEV"
PORTLIBS="$PS3DEV/portlibs/ppu"
OUT="$ROOT/build-ps3/padtest"
STAGE="$OUT/PS3_GAME"
mkdir -p "$OUT"

"$PS3DEV/ppu/bin/ppu-g++" -O2 -mcpu=cell -mhard-float -fno-exceptions -fno-rtti \
    -I"$PORTLIBS/include" -I"$PORTLIBS/include/SDL" -I"$PS3DEV/ppu/include" -I"$PS3DEV/ppu/include/simdmath" \
    "$ROOT/ports/ps3/padtest/padtest.cpp" -o "$OUT/padtest.elf" \
    -Wl,--start-group "$PORTLIBS/lib/libSDL.a" -lm -lrt -llv2 -lsysutil -lsysfs -lio -laudio -lgcm_sys -lrsx -Wl,--end-group

rm -rf "$STAGE"; mkdir -p "$STAGE/USRDIR"
"$PS3DEV/ppu/bin/ppu-strip" --strip-all "$OUT/padtest.elf" -o "$OUT/padtest_stripped.elf"
cp "$OUT/padtest_stripped.elf" "$OUT/padtest_linked.elf"
"$PS3DEV/bin/sprxlinker" "$OUT/padtest_linked.elf"
"$PS3DEV/bin/fself" "$OUT/padtest_linked.elf" "$STAGE/USRDIR/EBOOT.BIN"
"$PS3DEV/bin/sfo" -f "$PS3DEV/bin/sfo.xml" --title="RockBot PadTest" --appid="PADTEST01" "$STAGE/PARAM.SFO"
cp "$PS3DEV/bin/ICON0.PNG" "$STAGE/ICON0.PNG"
echo "PadTest package: $STAGE"
