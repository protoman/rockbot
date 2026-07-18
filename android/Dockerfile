# Rockbot Android build image: Android SDK/NDK + Pelya SDL port (commandergenius)
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    ANDROID_HOME=/opt/android-sdk \
    ANDROID_SDK_ROOT=/opt/android-sdk \
    PELYA_ANDROID_SDK=/opt/commandergenius \
    LANG=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
        openjdk-17-jdk-headless \
        unzip zip curl ca-certificates git \
        python3 rsync ffmpeg \
        build-essential cmake pkg-config \
        libncurses6 file less bsdutils \
    && rm -rf /var/lib/apt/lists/*

# Android command-line tools
ARG CMDLINE_TOOLS_VERSION=11076708
RUN mkdir -p ${ANDROID_HOME}/cmdline-tools \
    && curl -fsSL "https://dl.google.com/android/repository/commandlinetools-linux-${CMDLINE_TOOLS_VERSION}_latest.zip" \
        -o /tmp/cmdline-tools.zip \
    && unzip -q /tmp/cmdline-tools.zip -d ${ANDROID_HOME}/cmdline-tools \
    && mv ${ANDROID_HOME}/cmdline-tools/cmdline-tools ${ANDROID_HOME}/cmdline-tools/latest \
    && rm /tmp/cmdline-tools.zip

ENV PATH="${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/platform-tools:${PATH}"

# Platforms / build-tools / NDK (versions match current Rockbot AndroidAppSettings APP_PLATFORM=android-24+)
ARG ANDROID_PLATFORM=android-34
ARG ANDROID_BUILD_TOOLS=34.0.0
ARG ANDROID_NDK_VERSION=27.0.12077973

RUN yes | sdkmanager --licenses >/dev/null \
    && sdkmanager \
        "platform-tools" \
        "platforms;${ANDROID_PLATFORM}" \
        "build-tools;${ANDROID_BUILD_TOOLS}" \
        "ndk;${ANDROID_NDK_VERSION}"

ENV ANDROID_NDK_HOME=${ANDROID_HOME}/ndk/${ANDROID_NDK_VERSION} \
    ANDROID_BUILD_TOOLS=${ANDROID_BUILD_TOOLS} \
    PATH="${ANDROID_HOME}/build-tools/${ANDROID_BUILD_TOOLS}:${ANDROID_HOME}/ndk/${ANDROID_NDK_VERSION}:${PATH}"

# Pelya SDL-for-Android port
ARG PELYA_REPO=https://github.com/pelya/commandergenius.git
RUN git clone --depth 1 "${PELYA_REPO}" ${PELYA_ANDROID_SDK} \
    && cd ${PELYA_ANDROID_SDK} \
    && git submodule update --init --recursive || true

# Prefetch Gradle + plugin metadata so runtime builds don't need DNS.
ENV GRADLE_USER_HOME=/opt/gradle
RUN mkdir -p /opt/gradle \
    && printf 'sdk.dir=%s\n' "${ANDROID_HOME}" > ${PELYA_ANDROID_SDK}/project/local.properties \
    && cd ${PELYA_ANDROID_SDK}/project \
    && ./gradlew --no-daemon --version \
    && ./gradlew --no-daemon help

# Ensure Pelya / Gradle have a debug keystore available if they try to sign.
RUN mkdir -p /root/.android \
    && keytool -genkeypair -v \
        -keystore /root/.android/debug.keystore \
        -storepass android -keypass android \
        -alias androiddebugkey \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US"

WORKDIR /rockbot
COPY docker-build.sh /usr/local/bin/rockbot-android-build
RUN chmod +x /usr/local/bin/rockbot-android-build

ENTRYPOINT ["/usr/local/bin/rockbot-android-build"]
