#!/bin/bash
# Runs inside the rockbot-android Docker image.
set -e
set -o pipefail

REPO_ROOT="${REPO_ROOT:-/rockbot}"
PACKAGES_DIR="$REPO_ROOT/build/packages"
ANDROID_FILES="$PACKAGES_DIR/files/android"
GAME_DATA_ROOT="$REPO_ROOT/build"
PELYA_ANDROID_SDK="${PELYA_ANDROID_SDK:-/opt/commandergenius}"
ANDROID_HOME="${ANDROID_HOME:-/opt/android-sdk}"
ANDROID_NDK_HOME="${ANDROID_NDK_HOME:-}"
ANDROID_BUILD_TOOLS="${ANDROID_BUILD_TOOLS:-}"
ANDROID_KEYSTORE="${ANDROID_KEYSTORE:-/keys/release.keystore}"
ANDROID_KEY_ALIAS="${ANDROID_KEY_ALIAS:-alias_name}"

VERSION_NUMBER="${VERSION_NUMBER:-${1:-}}"
BUILD_TYPE="$(printf '%s' "${BUILD_TYPE:-${2:-release}}" | tr '[:upper:]' '[:lower:]')"
NO_SIGN="${NO_SIGN:-false}"

if [[ "$NO_SIGN" == "1" || "$NO_SIGN" == "true" || "$NO_SIGN" == "yes" ]]; then
    NO_SIGN=true
else
    NO_SIGN=false
fi

if [[ "$VERSION_NUMBER" != "1" && "$VERSION_NUMBER" != "2" ]]; then
    echo "Usage (container): VERSION_NUMBER=1|2 BUILD_TYPE=debug|release NO_SIGN=true rockbot-android-build"
    exit 1
fi

case "$BUILD_TYPE" in
    debug|release) ;;
    *)
        echo "Unknown BUILD_TYPE: $BUILD_TYPE"
        exit 1
        ;;
esac

if [[ -z "$ANDROID_NDK_HOME" ]]; then
    ANDROID_NDK_HOME="$(ls -1d "$ANDROID_HOME/ndk"/* 2>/dev/null | sort -V | tail -1)"
fi
if [[ -z "$ANDROID_BUILD_TOOLS" ]]; then
    ANDROID_BUILD_TOOLS="$(ls -1 "$ANDROID_HOME/build-tools" | sort -V | tail -1)"
fi

export ANDROID_HOME ANDROID_NDK_HOME PELYA_ANDROID_SDK
export PATH="$PELYA_ANDROID_SDK:$ANDROID_NDK_HOME:$ANDROID_HOME/build-tools/$ANDROID_BUILD_TOOLS:$PATH"

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
VERSION_CODE="$(printf '%s' "$VERSION_NAME" | tr -d '.')"
# Building 4 ABIs in Docker often OOMs; default to arm64-v8a. Override with ANDROID_ABIS.
ANDROID_ABIS="${ANDROID_ABIS:-arm64-v8a}"
# Cap ndk-build parallelism to reduce peak memory.
export NCPU="${NCPU:-2}"
export GRADLE_USER_HOME="${GRADLE_USER_HOME:-/opt/gradle}"
# Docker + volume mounts break Gradle's native FS watcher (error = 4).
# Also force plain console so progress bars don't scramble the log.
export CI=true
export GRADLE_OPTS="-Dorg.gradle.daemon=false -Dorg.gradle.vfs.watch=false -Dorg.gradle.console=plain ${GRADLE_OPTS:-}"
mkdir -p "$GRADLE_USER_HOME"
cat > "$GRADLE_USER_HOME/gradle.properties" <<'EOF'
org.gradle.daemon=false
org.gradle.vfs.watch=false
org.gradle.console=plain
org.gradle.parallel=false
org.gradle.caching=false
EOF
# Project-level props (do NOT sed build.sh — that breaks `if [ -x ./gradlew ]`).
GRADLE_PROPS="$PELYA_ANDROID_SDK/project/gradle.properties"
touch "$GRADLE_PROPS"
for key in org.gradle.vfs.watch org.gradle.console org.gradle.daemon; do
    grep -q "^${key}=" "$GRADLE_PROPS" 2>/dev/null && continue
    case "$key" in
        org.gradle.vfs.watch) echo "${key}=false" >> "$GRADLE_PROPS" ;;
        org.gradle.console) echo "${key}=plain" >> "$GRADLE_PROPS" ;;
        org.gradle.daemon) echo "${key}=false" >> "$GRADLE_PROPS" ;;
    esac
done

# If a previous broken run patched build.sh, restore gradlew checks.
if [[ -f "$PELYA_ANDROID_SDK/build.sh" ]] && grep -q '\[ -x \./gradlew --no-watch-fs' "$PELYA_ANDROID_SDK/build.sh"; then
    sed -i \
        -e 's|\[ -x \./gradlew --no-watch-fs --console=plain --no-daemon \]|[ -x ./gradlew ]|g' \
        -e 's|\./gradlew --no-watch-fs --console=plain --no-daemon |\./gradlew |g' \
        "$PELYA_ANDROID_SDK/build.sh"
fi

echo ">> [docker] Rockbot Android v${VERSION_NUMBER} (${VERSION_NAME}, ${BUILD_TYPE})"
echo "📦 PELYA_ANDROID_SDK=$PELYA_ANDROID_SDK"
echo "📦 ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
echo "📦 ANDROID_ABIS=$ANDROID_ABIS  NCPU=$NCPU"

NDK_PREBUILT="$(ls -1d "$ANDROID_NDK_HOME/toolchains/llvm/prebuilt"/* 2>/dev/null | head -1)"
NDK_SYSROOT_INC="$NDK_PREBUILT/sysroot/usr/include"
NDK_LIBCXX_INC="$ANDROID_NDK_HOME/sources/cxx-stl/llvm-libc++/include"
if [[ ! -d "$NDK_LIBCXX_INC" ]]; then
    NDK_LIBCXX_INC="$NDK_PREBUILT/sysroot/usr/include/c++/v1"
fi

APP_DIR="$PELYA_ANDROID_SDK/project/jni/application/rockbot"
WORK_DIR="$PACKAGES_DIR/Android"
OUT_APK="$PACKAGES_DIR/RockBot_Android_${VERSION_NAME}.apk"

echo "📁 Syncing sources into Pelya application/rockbot..."
mkdir -p "$APP_DIR"
rsync -a \
    --delete \
    --exclude='.git/' \
    --exclude='build/' \
    --exclude='editor/' \
    --exclude='/android/' \
    --exclude='docs/' \
    --exclude='.github/' \
    --exclude='*.o' \
    --exclude='*.apk' \
    --exclude='Makefile' \
    --exclude='.qmake.stash' \
    --exclude='build-*.sh' \
    "$REPO_ROOT/" "$APP_DIR/"

rsync -a "$ANDROID_FILES/" "$APP_DIR/"
safe_cp() {
    # Pelya may leave AndroidAppSettings.cfg as a dangling symlink.
    local src="$1"
    local dst="$2"
    mkdir -p "$(dirname "$dst")"
    if [[ -L "$dst" ]]; then
        rm -f "$dst"
    fi
    cp "$src" "$dst"
}

safe_cp "$ANDROID_FILES/AndroidAppSettings.cfg" "$APP_DIR/AndroidAppSettings.cfg"

echo "📁 Packaging game data (RockDroid${VERSION_NUMBER})..."
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/data/games" "$WORK_DIR/data/games/RockDroid${VERSION_NUMBER}/music/mp3"

for required in \
    "$GAME_DATA_ROOT/fonts" \
    "$GAME_DATA_ROOT/shared" \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}"
do
    if [[ ! -d "$required" ]]; then
        echo "Error: missing game data directory: $required"
        exit 1
    fi
done

rsync -a --exclude='.svn' "$GAME_DATA_ROOT/fonts" "$WORK_DIR/data/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/shared" "$WORK_DIR/data/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}" "$WORK_DIR/data/games/"

shopt -s nullglob
for filename in "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}/music/"*; do
    basename="${filename##*/}"
    case "$basename" in
        *.mod|*.s3m|*.xm|*.it|*.MOD|*.S3M|*.XM|*.IT)
            ffmpeg -y -i "$filename" -vn -ar 44100 -ac 2 -b:a 48k \
                "$WORK_DIR/data/games/RockDroid${VERSION_NUMBER}/music/mp3/${basename}.mp3" \
                </dev/null >/dev/null 2>&1
            ;;
    esac
done
shopt -u nullglob

rm -rf "$WORK_DIR/data/games/RockDroid${VERSION_NUMBER}/music/ogg"
rm -f "$WORK_DIR/data"/game*.sav "$WORK_DIR/data"/config_v*.sav

(
    cd "$WORK_DIR/data"
    zip -qr "../data_${VERSION_NAME}.zip" fonts games shared
)

mkdir -p "$APP_DIR/AndroidData"
rm -f "$APP_DIR/AndroidData/"*.zip
cp "$WORK_DIR/data_${VERSION_NAME}.zip" "$APP_DIR/AndroidData/"
cp "$ANDROID_FILES/logo_v${VERSION_NUMBER}.png" "$APP_DIR/AndroidData/logo.png"
cp "$ANDROID_FILES/icon_v${VERSION_NUMBER}.png" "$APP_DIR/icon.png"

echo "📝 Patching AndroidAppSettings.cfg..."
APP_CPPFLAGS="$(printf '%s' "-DANDROID -DHANDHELD -std=c++11 -I. -I${NDK_SYSROOT_INC} -I${NDK_LIBCXX_INC} -I./aux_tools -I./character -I./character/movement -I./file -I./file/format -I./file/v4 -I./graphic -I./objects -I./options -I./ports/android -I./scenes" | tr -s '[:space:]' ' ')"

if [[ "$VERSION_NUMBER" == "1" ]]; then
    APP_FULL_NAME="net.upperland.rockdroid"
else
    APP_FULL_NAME="net.upperland.rockdroid${VERSION_NUMBER}"
fi

export RB_VER="$VERSION_NUMBER" RB_FULL="$APP_FULL_NAME" RB_CODE="$VERSION_CODE" \
    RB_NAME="$VERSION_NAME" RB_CPP="$APP_CPPFLAGS" RB_ABIS="$ANDROID_ABIS"

python3 - "$APP_DIR/AndroidAppSettings.cfg" <<'PY'
import os, sys
path = sys.argv[1]
values = {
    "AppName": f'AppName="Rockbot {os.environ["RB_VER"]}"',
    "AppFullName": f'AppFullName={os.environ["RB_FULL"]}',
    "AppVersionCode": f'AppVersionCode={os.environ["RB_CODE"]}',
    "AppVersionName": f'AppVersionName="{os.environ["RB_NAME"]}"',
    "AppDataDownloadUrl": f'AppDataDownloadUrl="!Game Data|data_{os.environ["RB_NAME"]}.zip"',
    "AppCppflags": f'AppCppflags=\'{os.environ["RB_CPP"]}\'',
    "MultiABI": f"MultiABI='{os.environ['RB_ABIS']}'",
}
text = open(path, encoding="utf-8").read().splitlines(True)
out = []
for line in text:
    key = line.split("=", 1)[0] if "=" in line else None
    # skip commented MultiABI lines; replace the active one
    if key == "MultiABI" and line.lstrip().startswith("#"):
        out.append(line)
        continue
    out.append(values[key] + "\n" if key in values else line)
open(path, "w", encoding="utf-8").writelines(out)
PY

safe_cp "$APP_DIR/AndroidAppSettings.cfg" "$PELYA_ANDROID_SDK/AndroidAppSettings.cfg"

# changeAppSettings expects src to be a removable symlink (not a real directory).
SRC_LINK="$PELYA_ANDROID_SDK/project/jni/application/src"
rm -rf "$SRC_LINK"

echo "🛠️ Configuring Pelya application..."
(
    cd "$PELYA_ANDROID_SDK"
    ./changeAppSettings.sh rockbot -a
)

echo "🛠️ Building APK (${BUILD_TYPE})..."
# Pelya usage: build.sh [-z] [debug|release] [app-name]
# -z skips Pelya's zipalign/apksigner (we handle signing ourselves).
BUILD_LOG="$PACKAGES_DIR/android-build.log"
mkdir -p "$PACKAGES_DIR"
: > "$BUILD_LOG"
echo "📜 Mirroring build to Docker stdout + $BUILD_LOG"

# Line-by-line mirror: always print to container stdout (seen on host via
# docker attach) and flush to the log file. Avoids script/tee buffering issues.
export BUILD_LOG BUILD_TYPE PELYA_ANDROID_SDK
set +e
python3 -u - <<'PY'
import os, pty, select, subprocess, sys

log_path = os.environ["BUILD_LOG"]
cwd = os.environ["PELYA_ANDROID_SDK"]
cmd = ["./build.sh", "-z", os.environ["BUILD_TYPE"], "rockbot"]

master, slave = pty.openpty()
with open(log_path, "wb") as log:
    proc = subprocess.Popen(
        cmd,
        cwd=cwd,
        stdin=slave,
        stdout=slave,
        stderr=slave,
        close_fds=True,
    )
    os.close(slave)
    try:
        while True:
            if select.select([master], [], [], 0.2)[0]:
                try:
                    data = os.read(master, 4096)
                except OSError:
                    break
                if not data:
                    break
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
                log.write(data)
                log.flush()
            elif proc.poll() is not None:
                # Drain remaining output after process exit.
                while select.select([master], [], [], 0)[0]:
                    try:
                        data = os.read(master, 4096)
                    except OSError:
                        data = b""
                    if not data:
                        break
                    sys.stdout.buffer.write(data)
                    sys.stdout.buffer.flush()
                    log.write(data)
                    log.flush()
                break
    finally:
        os.close(master)
    raise SystemExit(proc.wait())
PY
BUILD_STATUS=$?
set -e
if [[ "$BUILD_STATUS" -ne 0 ]]; then
    echo "Error: Pelya build failed (exit $BUILD_STATUS). Last 80 lines:"
    tail -n 80 "$BUILD_LOG" || true
    exit "$BUILD_STATUS"
fi

echo "📦 Looking for generated APK..."
UNSIGNED_APK=""
case "$BUILD_TYPE" in
    release)
        for candidate in \
            "$PELYA_ANDROID_SDK/project/app/build/outputs/apk/release/app-release-unsigned.apk" \
            "$PELYA_ANDROID_SDK/project/app/build/outputs/apk/release/app-release.apk"
        do
            if [[ -f "$candidate" ]]; then
                UNSIGNED_APK="$candidate"
                break
            fi
        done
        ;;
    debug)
        for candidate in \
            "$PELYA_ANDROID_SDK/project/app/build/outputs/apk/debug/app-debug.apk" \
            "$PELYA_ANDROID_SDK/project/app/build/outputs/apk/debug/app-debug-unsigned.apk"
        do
            if [[ -f "$candidate" ]]; then
                UNSIGNED_APK="$candidate"
                break
            fi
        done
        ;;
esac

if [[ -z "$UNSIGNED_APK" || ! -f "$UNSIGNED_APK" ]]; then
    UNSIGNED_APK="$(find "$PELYA_ANDROID_SDK/project" -name '*.apk' 2>/dev/null | head -1 || true)"
fi
if [[ -z "$UNSIGNED_APK" || ! -f "$UNSIGNED_APK" ]]; then
    echo "Error: APK not found after build. Search results:"
    find "$PELYA_ANDROID_SDK/project" -name '*.apk' 2>/dev/null || true
    ls -laR "$PELYA_ANDROID_SDK/project/app/build/outputs" 2>/dev/null || true
    exit 1
fi

echo "📦 Found APK: $UNSIGNED_APK"

TEMP_APK="$PACKAGES_DIR/TEMP_RockBot_Android_${VERSION_NAME}.apk"
rm -f "$TEMP_APK" "$OUT_APK"
cp "$UNSIGNED_APK" "$TEMP_APK"
zip -d "$TEMP_APK" 'META-INF/*' 2>/dev/null || true

MAPPING_SRC="$PELYA_ANDROID_SDK/project/app/build/outputs/mapping/${BUILD_TYPE}/mapping.txt"
if [[ -f "$MAPPING_SRC" ]]; then
    cp "$MAPPING_SRC" "$PACKAGES_DIR/mappings_${VERSION_NAME}.txt"
fi

if $NO_SIGN || [[ "$BUILD_TYPE" == "debug" ]]; then
    cp "$TEMP_APK" "$OUT_APK"
    echo "⚠️ Skipping signing (--no-sign or debug build)"
else
    if [[ ! -f "$ANDROID_KEYSTORE" ]]; then
        echo "Error: keystore not found at $ANDROID_KEYSTORE"
        echo "Mount a keystore at /keys/release.keystore or pass --no-sign"
        exit 1
    fi
    echo "🔏 Signing APK..."
    zipalign -f -v 4 "$TEMP_APK" "$OUT_APK"
    if [[ -n "${ANDROID_KEYSTORE_PASS:-}" ]]; then
        apksigner sign \
            --ks "$ANDROID_KEYSTORE" \
            --ks-key-alias "$ANDROID_KEY_ALIAS" \
            --ks-pass "pass:${ANDROID_KEYSTORE_PASS}" \
            --v1-signing-enabled true \
            --v2-signing-enabled true \
            "$OUT_APK"
    else
        apksigner sign \
            --ks "$ANDROID_KEYSTORE" \
            --ks-key-alias "$ANDROID_KEY_ALIAS" \
            --v1-signing-enabled true \
            --v2-signing-enabled true \
            "$OUT_APK"
    fi
fi

rm -f "$TEMP_APK"

echo "✅ Build completed successfully"
echo "APK: $OUT_APK"
