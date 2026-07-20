#!/bin/bash
set -e
set -o pipefail

# Usage:
#   ./build-psp.sh 1                 → RockDroid 1 (Docker / pspdev)
#   ./build-psp.sh 2                 → RockDroid 2
#   ./build-psp.sh 1 --pull          → rebuild local image (refresh base + packages)
#
# Optional env:
#   ROCKBOT_PSP_IMAGE   Image tag (default: rockbot-psp:latest)
#   NCPU                make parallelism (default: 2)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
PSP_SCRIPT_DIR="$REPO_ROOT/ports/psp"
PACKAGES_DIR="$REPO_ROOT/build/packages"
IMAGE="${ROCKBOT_PSP_IMAGE:-rockbot-psp:latest}"

VERSION_NUMBER=""
PULL_IMAGE=false

for arg in "$@"; do
    case "$arg" in
        1|2) VERSION_NUMBER="$arg" ;;
        --pull) PULL_IMAGE=true ;;
        -h|--help)
            echo "Usage: $0 <1|2> [--pull]"
            echo
            echo "Builds the PSP EBOOT.PBP inside Docker (official pspdev image)."
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
    "$REPO_ROOT/Makefile.psp" \
    "$PSP_SCRIPT_DIR/docker-build.sh" \
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

chmod +x "$PSP_SCRIPT_DIR/docker-build.sh"
mkdir -p "$PACKAGES_DIR"

echo ">> Building Rockbot PSP v${VERSION_NUMBER} via Docker ($IMAGE)"

PLATFORM_ARGS=()
case "$(uname -m)" in
    arm64|aarch64) PLATFORM_ARGS=(--platform linux/amd64) ;;
esac

if $PULL_IMAGE || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "🛠️ Building local PSP image: $IMAGE (SDL2 packages baked in Dockerfile)"
    if $PULL_IMAGE; then
        docker pull "${PLATFORM_ARGS[@]}" pspdev/pspdev:latest || true
    fi
    docker build "${PLATFORM_ARGS[@]}" -t "$IMAGE" -f "$PSP_SCRIPT_DIR/Dockerfile" "$PSP_SCRIPT_DIR"
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
    -v "${PSP_SCRIPT_DIR}/docker-build.sh:/usr/local/bin/rockbot-psp-build:ro"
    -w /rockbot
)

if [[ -t 1 ]]; then
    DOCKER_ARGS+=(-t)
fi
if [[ -t 0 ]]; then
    DOCKER_ARGS+=(-i)
fi

echo "🛠️ Running build in container (live output below)..."
echo "📜 File log: build/packages/psp-build.log"

set +e
docker run "${PLATFORM_ARGS[@]}" "${DOCKER_ARGS[@]}" "$IMAGE" \
    /usr/local/bin/rockbot-psp-build
DOCKER_STATUS=$?
set -e

if [[ "$DOCKER_STATUS" -ne 0 ]]; then
    echo "Error: Docker PSP build failed (exit $DOCKER_STATUS)"
    if [[ -f "$PACKAGES_DIR/psp-build.log" ]]; then
        echo "---- tail of build/packages/psp-build.log ----"
        tail -n 80 "$PACKAGES_DIR/psp-build.log"
    fi
    exit "$DOCKER_STATUS"
fi

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
OUT_ZIP="$PACKAGES_DIR/RockBot_PSP_${VERSION_NAME}.zip"
GAME_DIR="Rockbot${VERSION_NUMBER}"
STAGE_DIR="$PACKAGES_DIR/psp_dist/$GAME_DIR"

if [[ ! -f "$OUT_ZIP" ]]; then
    echo "Error: expected zip was not produced:"
    echo "  $OUT_ZIP"
    ls -la "$PACKAGES_DIR" | sed -n '1,40p'
    exit 1
fi

# Install into PPSSPP memstick on this machine (macOS/Linux host).
PPSSPP_GAME="${HOME}/.config/ppsspp/PSP/GAME/${GAME_DIR}"
if [[ -d "${HOME}/.config/ppsspp/PSP/GAME" ]]; then
    echo "📦 Installing into PPSSPP memstick: $PPSSPP_GAME"
    rm -rf "$PPSSPP_GAME"
    mkdir -p "$PPSSPP_GAME"
    if [[ -d "$STAGE_DIR" ]]; then
        cp -a "$STAGE_DIR/" "$PPSSPP_GAME/"
    else
        unzip -qo "$OUT_ZIP" -d "${HOME}/.config/ppsspp/PSP/GAME"
    fi
    echo "Open with:"
    echo "  open -a PPSSPPSDL \"$PPSSPP_GAME/EBOOT.PBP\""
fi

echo "✅ Build completed successfully"
echo
echo "ZIP:"
echo "  $OUT_ZIP"
echo
echo "Install on PSP / PPSSPP:"
echo "  copy ${GAME_DIR}/ to ms0:/PSP/GAME/ (must include fonts/, shared/, games/)"
