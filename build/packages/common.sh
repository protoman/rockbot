# Shared helpers for the package scripts. Source it, do not execute it:
#     source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
#
# The release version lives in a single file, version_name.txt (format
# M.NN.PPP: major.minor.mini). Use ./release.sh to bump it.
#
# Provides:
#   PKG_DIR            directory of the package scripts (build/packages)
#   VERSION_FILE       path to version_name.txt
#   get_version        prints the current version
#   version_code       prints the Android versionCode for the current version
#   sync_version_header  writes the current version into file/version.h
#   stage_game_data DEST   copies fonts, shared and games/Rockbot<N> (without
#                          saves) into DEST; needs $version_number (see choose_game)
#   choose_game [1|2]  sets $version_number (the game: 1 = Rockbot 1,
#                      2 = Rockbot 2) and $VERSIONNAME; uses the argument if
#                      given, otherwise asks.

PKG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION_FILE="${ROCKBOT_VERSION_FILE:-$PKG_DIR/version_name.txt}"

get_version() {
    local v
    v="$(tr -d '[:space:]' < "$VERSION_FILE" 2>/dev/null)" || true
    if ! [[ "$v" =~ ^[0-9]+\.[0-9]{2}\.[0-9]{3}$ ]]; then
        echo "Invalid or missing version in $VERSION_FILE (expected M.NN.PPP, got '$v')" >&2
        return 1
    fi
    printf '%s\n' "$v"
}

# Android versionCode must always be greater than the previously published one.
# M.NN.PPP -> M*100000 + NN*1000 + PPP, e.g. 2.22.102 -> 222102 (identical to the
# old "remove the dots" value, but still increasing at 3.00.000 -> 300000).
# ANDROID_MIN_VERSION_CODE is the highest code ever published; a version that
# would map at or below it (and so could not be published) is refused.
ANDROID_MIN_VERSION_CODE=222102
version_code() {
    local v maj min mini code
    v="$(get_version)" || return 1
    IFS=. read -r maj min mini <<< "$v"
    code=$((10#$maj * 100000 + 10#$min * 1000 + 10#$mini))
    if [ "$code" -lt "$ANDROID_MIN_VERSION_CODE" ]; then
        echo "Version $v gives Android versionCode $code, lower than the already published $ANDROID_MIN_VERSION_CODE" >&2
        return 1
    fi
    printf '%s\n' "$code"
}

# file/version.h is what the game (and the Android build) read; keep it equal to
# version_name.txt. Only rewritten when it differs.
VERSION_HEADER="${ROCKBOT_VERSION_HEADER:-$PKG_DIR/../../file/version.h}"
sync_version_header() {
    local v current
    v="$(get_version)" || return 1
    [ -f "$VERSION_HEADER" ] || { echo "Missing $VERSION_HEADER" >&2; return 1; }
    current="$(sed -nE 's/^\s*#\s*define\s+VERSION_NUMBER\s+"([^"]*)".*/\1/p' "$VERSION_HEADER")"
    if [ "$current" != "$v" ]; then
        sed -i -E "s/^(\s*#\s*define\s+VERSION_NUMBER\s+)\"[^\"]*\"/\1\"$v\"/" "$VERSION_HEADER"
        echo "Updated $VERSION_HEADER: $current -> $v"
    fi
}

stage_game_data() {
    local dest="$1"
    mkdir -p "$dest/games"
    rsync -a --exclude=.svn "$PKG_DIR/../fonts" "$dest/"
    rsync -a --exclude=.svn "$PKG_DIR/../shared" "$dest/"
    rsync -a --exclude=.svn "$PKG_DIR/../games/Rockbot$version_number" "$dest/games/"
    find "$dest" \( -name 'game*.sav' -o -name 'config*.sav' -o -name 'errors.log' \) -delete
}

choose_game() {
    local choice="${1:-}"
    if [ -z "$choice" ]; then
        if [ ! -t 0 ]; then
            echo "Game number required (1 or 2) when not running interactively" >&2
            return 1
        fi
        echo "Which game are you building?"
        echo "  1) Rockbot 1"
        echo "  2) Rockbot 2"
        read -r -p "Enter 1 or 2: " choice
    fi
    if [ "$choice" != "1" ] && [ "$choice" != "2" ]; then
        echo "Invalid game number '$choice' (use 1 or 2)" >&2
        return 1
    fi
    version_number="$choice"
    VERSIONNAME="$(get_version)" || return 1
    sync_version_header || return 1
    echo "Building Rockbot $version_number, version $VERSIONNAME"
}
