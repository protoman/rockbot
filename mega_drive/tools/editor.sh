#!/bin/sh
# Launch the stage editor on the port's own game data (data/games/RockDroid1).
# The editor is the PC Qt editor, copied into mega_drive/editor and pointed at
# this tree; edits land in data/games/RockDroid1, which is what every converter
# reads (the parent rockbot/build/games copy is no longer used).
set -e
HERE=$(cd "$(dirname "$0")/.." && pwd)
export ROCKBOT_EDITOR_PATH="$HERE/data"
exec "$HERE/editor/editor" "$@"
