#!/usr/bin/env bash
# General release script: asks for the version bump (mini/minor/major, see release.sh), then builds
# and packages every platform for the chosen game(s). A failure in one target does not stop the
# others; a summary (and one log per build) is written at the end.
#
# Usage:
#   ./release_all.sh                               interactive: asks bump and game(s)
#   ./release_all.sh --bump minor --game both      fully automatic
#   ./release_all.sh --bump none --game 1 --only psp,ps3
#
# Options:
#   --bump mini|minor|major|none   version part to increase (none = keep the current version)
#   --game 1|2|both                which game(s) to release
#   --only a,b,c                   build only these targets
#   --skip a,b,c                   build everything except these targets
#   --list                         list the targets and exit
#   --dry-run                      show what would run, change nothing
#   -h, --help
#
# Targets: ps2 psp ps3 dingux pocketgo wii linux-targz linux-deb linux-appimage
#          windows-x64 windows-i686 android raspberry-armhf raspberry-arm64
#
# Output: build/packages/Rockbot<N>_<Platform>_<version>.*   logs: build/packages/release_logs/<version>/
set -uo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"

# name | command (the game number is appended)
TARGETS=(
    "ps2|./PS2.sh"
    "psp|./PSP.sh"
    "ps3|./PS3.sh"
    "dingux|./Dingux.sh"
    "pocketgo|./PocketGo.sh"
    "wii|./Wii.sh"
    "linux-targz|./linux.sh"
    "linux-deb|./deb.sh"
    "linux-appimage|./appimage.sh"
    "windows-x64|./win32.sh @GAME@ sdl2 x86_64"
    "windows-i686|./win32.sh @GAME@ sdl2 i686"
    "android|./Android.sh @GAME@"
    "raspberry-armhf|./Raspberry.sh @GAME@ armhf"
    "raspberry-arm64|./Raspberry.sh @GAME@ arm64"
)

BUMP=""; GAMES=""; ONLY=""; SKIP=""; DRY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --bump) BUMP="${2:-}"; shift 2 ;;
        --game) GAMES="${2:-}"; shift 2 ;;
        --only) ONLY="${2:-}"; shift 2 ;;
        --skip) SKIP="${2:-}"; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        --list) for t in "${TARGETS[@]}"; do echo "${t%%|*}"; done; exit 0 ;;
        -h|--help) sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown option: $1 (see --help)" >&2; exit 1 ;;
    esac
done

in_list() { case ",$2," in *",$1,"*) return 0 ;; esac; return 1; }

# --- targets ---------------------------------------------------------------------------------
SELECTED=()
for t in "${TARGETS[@]}"; do
    name="${t%%|*}"
    if [ -n "$ONLY" ] && ! in_list "$name" "$ONLY"; then continue; fi
    if [ -n "$SKIP" ] && in_list "$name" "$SKIP"; then continue; fi
    SELECTED+=("$t")
done
for want in ${ONLY//,/ } ${SKIP//,/ }; do
    found=0
    for t in "${TARGETS[@]}"; do [ "${t%%|*}" = "$want" ] && found=1; done
    [ "$found" = 1 ] || { echo "Unknown target '$want' (see --list)" >&2; exit 1; }
done
[ ${#SELECTED[@]} -gt 0 ] || { echo "No targets selected" >&2; exit 1; }

# --- games -----------------------------------------------------------------------------------
if [ -z "$GAMES" ]; then
    if [ -t 0 ]; then
        echo "Which game(s) do you want to release?"
        echo "  1) Rockbot 1"
        echo "  2) Rockbot 2"
        echo "  3) both"
        read -r -p "Enter 1, 2 or 3: " choice
        case "$choice" in 1) GAMES=1 ;; 2) GAMES=2 ;; 3) GAMES=both ;; *) echo "Invalid choice" >&2; exit 1 ;; esac
    else
        echo "--game 1|2|both is required when not running interactively" >&2; exit 1
    fi
fi
case "$GAMES" in
    1|2) GAME_LIST=("$GAMES") ;;
    both|1,2) GAME_LIST=(1 2) ;;
    *) echo "Invalid --game '$GAMES' (use 1, 2 or both)" >&2; exit 1 ;;
esac

# --- version ---------------------------------------------------------------------------------
case "$BUMP" in
    "")
        if [ -t 0 ]; then
            if [ "$DRY" = 1 ]; then echo "(dry run: would ask for the version bump)"; else ./release.sh || exit 1; fi
        else
            echo "--bump mini|minor|major|none is required when not running interactively" >&2; exit 1
        fi ;;
    none) echo "Keeping version $(get_version)" ;;
    mini|minor|major)
        if [ "$DRY" = 1 ]; then echo "(dry run: would bump the $BUMP version from $(get_version))"; else ./release.sh "$BUMP" || exit 1; fi ;;
    *) echo "Invalid --bump '$BUMP' (mini, minor, major or none)" >&2; exit 1 ;;
esac
VERSION="$(get_version)" || exit 1
[ "$DRY" = 1 ] || sync_version_header
LOG_DIR="$SCRIPT_DIR/release_logs/$VERSION"
mkdir -p "$LOG_DIR"

echo
echo "Release $VERSION: games ${GAME_LIST[*]}, ${#SELECTED[@]} targets: $(for t in "${SELECTED[@]}"; do printf '%s ' "${t%%|*}"; done)"
echo

# --- build everything ---------------------------------------------------------------------------
RESULTS=()
START_ALL=$(date +%s)
for game in "${GAME_LIST[@]}"; do
    for t in "${SELECTED[@]}"; do
        name="${t%%|*}"; cmd="${t#*|}"
        case "$cmd" in
            *@GAME@*) cmd="${cmd//@GAME@/$game}" ;;
            *) cmd="$cmd $game" ;;
        esac
        label="Rockbot $game / $name"
        if [ "$DRY" = 1 ]; then echo "[dry run] $label: $cmd"; continue; fi
        log="$LOG_DIR/${name}_rockbot$game.log"
        echo "==> $label ..."
        start=$(date +%s)
        if $cmd > "$log" 2>&1 < /dev/null; then
            status="OK"
        else
            status="FAILED"
        fi
        elapsed=$(( $(date +%s) - start ))
        pkg_file="$(grep -E '^(Package|Installer|Folder zip):' "$log" | sed -E 's/^[A-Za-z ]+: //' | head -2 | xargs -r -n1 basename | tr '\n' ' ')"
        echo "    $status in ${elapsed}s ${pkg_file:+→ $pkg_file}"
        [ "$status" = OK ] || echo "    log: $log (last lines: $(tail -n 2 "$log" | tr '\n' ' ' | cut -c1-200))"
        RESULTS+=("$status|$label|${elapsed}s|$pkg_file")
    done
done

[ "$DRY" = 1 ] && exit 0

# --- summary -----------------------------------------------------------------------------------
echo
echo "================ Release $VERSION summary ($(( $(date +%s) - START_ALL ))s) ================"
failed=0
for r in "${RESULTS[@]}"; do
    IFS='|' read -r status label elapsed pkg_file <<< "$r"
    printf '%-7s %-34s %-7s %s\n' "$status" "$label" "$elapsed" "$pkg_file"
    [ "$status" = OK ] || failed=$((failed + 1))
done | tee "$LOG_DIR/summary.txt"
failed=$(grep -c '^FAILED' "$LOG_DIR/summary.txt" || true)
echo "Logs: $LOG_DIR"
if [ "$failed" -gt 0 ]; then
    echo "$failed build(s) failed."
    exit 1
fi
echo "All builds succeeded."
