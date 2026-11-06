#!/usr/bin/env bash
# Bumps the release version. Shows the latest version, asks which part to
# increase and writes the new one to version_name.txt (and file/version.h,
# which the game and the Android build read).
#
# Version format: M.NN.PPP  (M = major, NN = minor, PPP = mini)
#   mini   -> M.NN.(PPP+1)
#   minor  -> M.(NN+1).000
#   major  -> (M+1).00.000
#
# Usage: ./release.sh [mini|minor|major]
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

current="$(get_version)"
IFS=. read -r major minor mini <<< "$current"
major=$((10#$major)); minor=$((10#$minor)); mini=$((10#$mini))

fmt() { printf '%d.%02d.%03d' "$1" "$2" "$3"; }
next_mini="$(fmt "$major" "$minor" $((mini + 1)))"
next_minor="$(fmt "$major" $((minor + 1)) 0)"
next_major="$(fmt $((major + 1)) 0 0)"

echo "Latest version: $current"

part="${1:-}"
if [ -z "$part" ]; then
    echo "Which part do you want to increase?"
    echo "  1) mini   -> $next_mini"
    echo "  2) minor  -> $next_minor"
    echo "  3) major  -> $next_major"
    read -r -p "Enter 1, 2 or 3: " choice
    case "$choice" in
        1|mini)  part=mini ;;
        2|minor) part=minor ;;
        3|major) part=major ;;
        *) echo "Invalid choice '$choice'" >&2; exit 1 ;;
    esac
fi

case "$part" in
    mini)  new="$next_mini" ;;
    minor) new="$next_minor" ;;
    major) new="$next_major" ;;
    *) echo "Usage: $0 [mini|minor|major]" >&2; exit 1 ;;
esac

# PPP has 3 digits and NN has 2.
if [ $((mini + 1)) -gt 999 ] && [ "$part" = mini ]; then
    echo "Mini would exceed 999; bump the minor version instead." >&2; exit 1
fi
if [ $((minor + 1)) -gt 99 ] && [ "$part" = minor ]; then
    echo "Minor would exceed 99; bump the major version instead." >&2; exit 1
fi

if [ -t 0 ] && [ -z "${1:-}" ]; then
    read -r -p "Release $part version: $current -> $new. Continue? [y/N] " ok
    case "$ok" in [yY]|[yY][eE][sS]) ;; *) echo "Aborted."; exit 1 ;; esac
fi

printf '%s\n' "$new" > "$VERSION_FILE"
sync_version_header
echo "Version is now $new"
echo "Updated: $VERSION_FILE"
echo "Android versionCode: $(version_code)"
