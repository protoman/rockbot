# Android build image: JDK, Android SDK, NDK and the commandergenius (SDL for Android) project,
# built from scratch. The layout matches build/packages/android_build.sh (ANDROID_BASE):
#   $ANDROID_BASE/commandergenius, $ANDROID_BASE/SDK, $ANDROID_BASE/android-ndk-r29
# Local changes to commandergenius (NDK 28+ fix, Rockbot button images, manifest/Globals tweaks) are
# applied from ports/android/overlay/.
#
#   build/docker/build_images.sh android
FROM ubuntu:26.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates openjdk-21-jdk-headless git wget unzip zip xz-utils rsync ffmpeg imagemagick \
        build-essential cmake make pkg-config python3 file curl \
 && rm -rf /var/lib/apt/lists/*

ARG CMDLINE_TOOLS=14742923
ARG NDK_VERSION=r29
ARG COMMANDERGENIUS_REPO=https://github.com/protoman/commandergenius
ARG COMMANDERGENIUS_COMMIT=c5f29ae26a16c7b313cffd8e6cb9574fc4f8b08a
ENV ANDROID_BASE=/opt/android
ENV ANDROID_HOME=/opt/android/SDK
ENV ANDROID_SDK_ROOT=/opt/android/SDK
ENV GRADLE_USER_HOME=/opt/gradle-home
WORKDIR /opt/android

# Android SDK
RUN mkdir -p SDK/cmdline-tools && cd SDK/cmdline-tools \
 && wget -q https://dl.google.com/android/repository/commandlinetools-linux-${CMDLINE_TOOLS}_latest.zip -O t.zip \
 && unzip -q t.zip && mv cmdline-tools latest && rm t.zip \
 && yes | SDK/../../latest/bin/sdkmanager --sdk_root=/opt/android/SDK --licenses >/dev/null 2>&1 || true
RUN yes | /opt/android/SDK/cmdline-tools/latest/bin/sdkmanager --sdk_root=/opt/android/SDK \
        "platform-tools" "platforms;android-36" "build-tools;36.1.0" "build-tools;35.0.0"

# NDK
RUN wget -q https://dl.google.com/android/repository/android-ndk-${NDK_VERSION}-linux.zip -O n.zip \
 && unzip -q n.zip && rm n.zip && ls -d android-ndk-*

# commandergenius: fork pinned to a commit, no submodules (Rockbot only needs the SDL 1.2 stack in-tree)
COPY overlay /tmp/overlay
RUN git init -q commandergenius && cd commandergenius \
 && git fetch -q --depth 1 ${COMMANDERGENIUS_REPO} ${COMMANDERGENIUS_COMMIT} && git checkout -q FETCH_HEAD \
 && git apply --whitespace=nowarn /tmp/overlay/0001-fix-ndk28.patch 2>/dev/null || git apply /tmp/overlay/0001-fix-ndk28.patch \
 && git apply /tmp/overlay/0002-rockbot-buttons-and-config.patch \
 && cp /tmp/overlay/extra-files/*.png project/themes/SimpleTheme/ \
 && rm -rf /tmp/overlay

# Submodules the SDL 1.2 build still needs: the SDL2 family with its external libraries (jpeg/png,
# ogg/vorbis/... under sdl2_*/external), iconv and shmem. The ~20 application submodules are skipped.
RUN cd commandergenius \
 && git submodule update --init --depth 1 project/jni/sdl2 project/jni/sdl2_image project/jni/sdl2_ttf \
        project/jni/sdl2_mixer project/jni/iconv/src project/jni/shmem \
 && for d in sdl2 sdl2_image sdl2_ttf sdl2_mixer; do git -C project/jni/$d submodule update --init --depth 1 --recursive; done \
 && chmod -R a+rwX /opt/android/commandergenius

# Gradle distribution used by the project's wrapper, downloaded once here
RUN mkdir -p ${GRADLE_USER_HOME} && cd commandergenius/project && ./gradlew --version >/dev/null \
 && chmod -R a+rwX ${GRADLE_USER_HOME} /opt/android/commandergenius
ENV PATH=/opt/android/SDK/build-tools/36.1.0:/opt/android/SDK/platform-tools:$PATH
