#!/usr/bin/env bash
# Android APK. Native build by default (commandergenius/NDK/Gradle on this machine, see
# android_build.sh); set ANDROID_BUILD=docker to use the Docker build (../../build-android.sh).
# Usage: ./Android.sh [1|2] [--no-sign]     (asks which game if omitted)
# Creates Rockbot<N>_Android_<version>.apk (signed when the keystore password is available,
# see android_build.sh).
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

if [ "${ANDROID_BUILD:-native}" = "docker" ]; then
    ../../build-android.sh "$version_number" release "${extra[@]+"${extra[@]}"}"
else
    ./android_build.sh "$version_number" "${extra[@]+"${extra[@]}"}"
fi
echo "Package: $PWD/Rockbot${version_number}_Android_${VERSIONNAME}.apk"
