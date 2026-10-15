#!/bin/bash
set -e
set -o pipefail

# Usage:
#   ./build-switch.sh 1                 → RockDroid 1 (Docker / devkitA64)
#   ./build-switch.sh 2                 → RockDroid 2
#   ./build-switch.sh 1 --pull          → rebuild local image (refresh base + packages)
#
# Optional env:
#   ROCKBOT_SWITCH_IMAGE   Image tag (default: rockbot-switch:latest)
#   NCPU                   make parallelism (default: 2)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SWITCH_SCRIPT_DIR="$REPO_ROOT/ports/switch"
PACKAGES_DIR="$REPO_ROOT/build/packages"
IMAGE="${ROCKBOT_SWITCH_IMAGE:-rockbot-switch:latest}"

VERSION_NUMBER=""
PULL_IMAGE=false

for arg in "$@"; do
    case "$arg" in
        1|2) VERSION_NUMBER="$arg" ;;
        --pull) PULL_IMAGE=true ;;
        -h|--help)
            echo "Usage: $0 <1|2> [--pull]"
            echo
            echo "Builds rockbot.nro inside Docker (devkitpro/devkita64)."
            echo "Requires Docker. Game data must exist under build/{fonts,shared,games}."
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 <1|2> [--pull]"
            exit 1
            ;;
    esac
done

if [[ "$VERSION_NUMBER" != "1" && "$VERSION_NUMBER" != "2" ]]; then
    echo "Usage: $0 <1|2> [--pull]"
    exit 1
fi

if ! command -v docker >/dev/null 2>&1; then
    echo "Error: docker is required"
    exit 1
fi

for required in \
    "$REPO_ROOT/Makefile.switch" \
    "$SWITCH_SCRIPT_DIR/docker-build.sh" \
    "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt" \
    "$REPO_ROOT/build/fonts" \
    "$REPO_ROOT/build/shared" \
    "$REPO_ROOT/build/games/RockDroid${VERSION_NUMBER}"
do
    if [[ ! -e "$required" ]]; then
        echo "Error: missing required path: $required"
        exit 1
    fi
done

chmod +x "$SWITCH_SCRIPT_DIR/docker-build.sh"
mkdir -p "$PACKAGES_DIR"

echo ">> Building Rockbot Switch v${VERSION_NUMBER} via Docker ($IMAGE)"

PLATFORM_ARGS=()
case "$(uname -m)" in
    arm64|aarch64) PLATFORM_ARGS=(--platform linux/amd64) ;;
esac

if $PULL_IMAGE || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "🛠️ Building local Switch image: $IMAGE (SDL2 packages baked in Dockerfile)"
    if $PULL_IMAGE; then
        docker pull "${PLATFORM_ARGS[@]}" devkitpro/devkita64:latest || true
    fi
    docker build "${PLATFORM_ARGS[@]}" -t "$IMAGE" -f "$SWITCH_SCRIPT_DIR/Dockerfile" "$SWITCH_SCRIPT_DIR"
else
    echo "📦 Using local image: $IMAGE (rebuild with --pull to refresh packages)"
fi

DOCKER_ARGS=(
    --rm
    --privileged
    --dns 8.8.8.8
    --dns 1.1.1.1
    -e "VERSION_NUMBER=${VERSION_NUMBER}"
    -e "NCPU=${NCPU:-2}"
    -e "REPO_ROOT=/rockbot"
    -e "TERM=${TERM:-xterm-256color}"
    -e "PYTHONUNBUFFERED=1"
    -v "${REPO_ROOT}:/rockbot"
    -v "${SWITCH_SCRIPT_DIR}/docker-build.sh:/usr/local/bin/rockbot-switch-build:ro"
    -w /rockbot
)

if [[ -t 1 ]]; then
    DOCKER_ARGS+=(-t)
fi
if [[ -t 0 ]]; then
    DOCKER_ARGS+=(-i)
fi

echo "🛠️ Running build in container (live output below)..."
echo "📜 File log: build/packages/switch-build.log"

set +e
docker run "${PLATFORM_ARGS[@]}" "${DOCKER_ARGS[@]}" "$IMAGE" \
    /usr/local/bin/rockbot-switch-build
DOCKER_STATUS=$?
set -e

if [[ "$DOCKER_STATUS" -ne 0 ]]; then
    echo "Error: Docker Switch build failed (exit $DOCKER_STATUS)"
    if [[ -f "$PACKAGES_DIR/switch-build.log" ]]; then
        echo "---- tail of build/packages/switch-build.log ----"
        tail -n 80 "$PACKAGES_DIR/switch-build.log"
    fi
    exit "$DOCKER_STATUS"
fi

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
OUT_ZIP="$PACKAGES_DIR/RockBot_Switch_${VERSION_NAME}.zip"

if [[ ! -f "$OUT_ZIP" ]]; then
    echo "Error: expected zip was not produced:"
    echo "  $OUT_ZIP"
    ls -la "$PACKAGES_DIR" | sed -n '1,40p'
    exit 1
fi

echo "✅ Build completed successfully"
echo
echo "ZIP:"
echo "  $OUT_ZIP"
echo
echo "Install on Switch (Atmosphere / hbmenu):"
echo "  copy Rockbot${VERSION_NUMBER}/ to sdmc:/switch/ (must include fonts/, shared/, games/ next to rockbot.nro)"
