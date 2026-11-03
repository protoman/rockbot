#!/usr/bin/env bash
# Android APK (Docker build, see ../../build-android.sh): creates Rockbot<N>_Android_<version>.apk
# Usage: ./Android.sh [1|2] [debug|release] [--no-sign]    (asks which game if omitted)
# Release builds are signed with ~/.android/my-release-key.keystore (override with ANDROID_KEYSTORE,
# ANDROID_KEY_ALIAS, ANDROID_KEYSTORE_PASS); without a keystore the APK is left unsigned.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"

game="" ; extra=()
for arg in "$@"; do
    case "$arg" in
        1|2) game="$arg" ;;
        *) extra+=("$arg") ;;
    esac
done
choose_game "$game" || exit 1

../../build-android.sh "$version_number" "${extra[@]+"${extra[@]}"}"
echo "Package: $PWD/Rockbot${version_number}_Android_${VERSIONNAME}.apk"
