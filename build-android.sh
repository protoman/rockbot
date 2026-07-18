#!/bin/bash
set -e
set -o pipefail

# Usage:
#   ./build-android.sh 1                 → RockDroid 1, release (Docker)
#   ./build-android.sh 2 debug           → RockDroid 2, debug
#   ./build-android.sh 1 release --no-sign
#   ./build-android.sh 1 --rebuild-image → force rebuild Docker image
#
# Optional env:
#   ROCKBOT_ANDROID_IMAGE   Image tag (default: rockbot-android:latest)
#   ANDROID_KEYSTORE        Host keystore path (mounted at /keys/release.keystore)
#   ANDROID_KEY_ALIAS       Key alias (default: alias_name)
#   ANDROID_KEYSTORE_PASS   Keystore password
#   ANDROID_ABIS            NDK ABIs (default: arm64-v8a)
#   NCPU                    ndk-build parallelism (default: 2)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
ANDROID_DIR="$REPO_ROOT/android"
PACKAGES_DIR="$REPO_ROOT/build/packages"
IMAGE="${ROCKBOT_ANDROID_IMAGE:-rockbot-android:latest}"

VERSION_NUMBER=""
BUILD_TYPE="release"
NO_SIGN=false
REBUILD_IMAGE=false

for arg in "$@"; do
    case "$arg" in
        1|2) VERSION_NUMBER="$arg" ;;
        debug|release|DEBUG|RELEASE) BUILD_TYPE="$(printf '%s' "$arg" | tr '[:upper:]' '[:lower:]')" ;;
        --no-sign) NO_SIGN=true ;;
        --rebuild-image) REBUILD_IMAGE=true ;;
        -h|--help)
            echo "Usage: $0 <1|2> [debug|release] [--no-sign] [--rebuild-image]"
            echo
            echo "Builds the Android APK inside Docker (Android SDK/NDK + Pelya SDL port)."
            echo "Requires Docker. Game data must exist under build/{fonts,shared,games}."
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 <1|2> [debug|release] [--no-sign] [--rebuild-image]"
            exit 1
            ;;
    esac
done

if [[ "$VERSION_NUMBER" != "1" && "$VERSION_NUMBER" != "2" ]]; then
    echo "Usage: $0 <1|2> [debug|release] [--no-sign] [--rebuild-image]"
    exit 1
fi

if ! command -v docker >/dev/null 2>&1; then
    echo "Error: docker is required"
    exit 1
fi

for required in \
    "$PACKAGES_DIR/files/android/AndroidAppSettings.cfg" \
    "$REPO_ROOT/build/fonts" \
    "$REPO_ROOT/build/shared" \
    "$REPO_ROOT/build/games/RockDroid${VERSION_NUMBER}"
do
    if [[ ! -e "$required" ]]; then
        echo "Error: missing required path: $required"
        exit 1
    fi
done

echo ">> Building Rockbot Android v${VERSION_NUMBER} (${BUILD_TYPE}) via Docker"

PLATFORM_ARGS=()
case "$(uname -m)" in
    arm64|aarch64) PLATFORM_ARGS=(--platform linux/amd64) ;;
esac

need_build=false
if $REBUILD_IMAGE; then
    need_build=true
elif ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    need_build=true
fi

mkdir -p "$PACKAGES_DIR"

if $need_build; then
    echo "🛠️ Building Docker image: $IMAGE ${PLATFORM_ARGS[*]}"
    # --progress=plain streams every build step to the terminal
    docker build "${PLATFORM_ARGS[@]}" --progress=plain -t "$IMAGE" "$ANDROID_DIR"
else
    echo "📦 Using existing image: $IMAGE"
fi

DOCKER_ARGS=(
    --rm
    --dns 8.8.8.8
    --dns 1.1.1.1
    -e "VERSION_NUMBER=${VERSION_NUMBER}"
    -e "BUILD_TYPE=${BUILD_TYPE}"
    -e "NO_SIGN=${NO_SIGN}"
    -e "ANDROID_KEY_ALIAS=${ANDROID_KEY_ALIAS:-alias_name}"
    -e "ANDROID_ABIS=${ANDROID_ABIS:-arm64-v8a}"
    -e "NCPU=${NCPU:-2}"
    -e "GRADLE_USER_HOME=/opt/gradle"
    -e "PYTHONUNBUFFERED=1"
    -e "TERM=${TERM:-xterm-256color}"
    -v "${REPO_ROOT}:/rockbot"
    -v "${ANDROID_DIR}/docker-build.sh:/usr/local/bin/rockbot-android-build:ro"
)

# Attach a TTY when possible so container logs stream live (line-buffered).
# Never pipe docker through tee — that hides output until buffers flush.
if [[ -t 1 ]]; then
    DOCKER_ARGS+=(-t)
fi
if [[ -t 0 ]]; then
    DOCKER_ARGS+=(-i)
fi

if [[ -n "${ANDROID_KEYSTORE_PASS:-}" ]]; then
    DOCKER_ARGS+=(-e "ANDROID_KEYSTORE_PASS=${ANDROID_KEYSTORE_PASS}")
fi

ANDROID_KEYSTORE="${ANDROID_KEYSTORE:-$HOME/.android/my-release-key.keystore}"
if [[ -f "$ANDROID_KEYSTORE" ]]; then
    DOCKER_ARGS+=(-v "${ANDROID_KEYSTORE}:/keys/release.keystore:ro")
    DOCKER_ARGS+=(-e "ANDROID_KEYSTORE=/keys/release.keystore")
elif ! $NO_SIGN && [[ "$BUILD_TYPE" == "release" ]]; then
    echo "⚠️ Keystore not found ($ANDROID_KEYSTORE); forcing --no-sign"
    DOCKER_ARGS+=(-e "NO_SIGN=true")
fi

echo "🛠️ Running build in container (live output below)..."
echo "📜 File log: build/packages/android-build.log"

set +e
docker run "${PLATFORM_ARGS[@]}" "${DOCKER_ARGS[@]}" "$IMAGE"
DOCKER_STATUS=$?
set -e

if [[ "$DOCKER_STATUS" -ne 0 ]]; then
    echo "Error: Docker Android build failed (exit $DOCKER_STATUS)"
    if [[ -f "$PACKAGES_DIR/android-build.log" ]]; then
        echo "---- tail of build/packages/android-build.log ----"
        tail -n 80 "$PACKAGES_DIR/android-build.log"
    fi
    exit "$DOCKER_STATUS"
fi

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
OUT_APK="$PACKAGES_DIR/RockBot_Android_${VERSION_NAME}.apk"

if [[ ! -f "$OUT_APK" ]]; then
    echo "Error: expected APK was not produced:"
    echo "  $OUT_APK"
    echo "Contents of build/packages:"
    ls -la "$PACKAGES_DIR" | sed -n '1,40p'
    exit 1
fi

echo "✅ Build completed successfully"
echo
echo "APK:"
echo "  $OUT_APK"
echo
echo "Install:"
echo "  adb install -r \"$OUT_APK\""
