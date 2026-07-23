FROM devkitpro/devkita64:latest

# Zip/rsync for packaging (image base varies; try apt then pacman).
RUN (command -v apt-get >/dev/null && apt-get update && apt-get install -y --no-install-recommends rsync zip ca-certificates && rm -rf /var/lib/apt/lists/*) \
 || (command -v pacman >/dev/null && pacman -Sy --noconfirm rsync zip ca-certificates) \
 || true

# Ensure Switch SDL2 + common deps are present (skip if already installed).
RUN dkp-pacman -Sy --noconfirm || true \
 && for pkg in \
        switch-sdl2 switch-sdl2_image switch-sdl2_mixer switch-sdl2_ttf switch-sdl2_gfx \
        switch-libpng switch-libjpeg-turbo switch-freetype switch-zlib switch-bzip2 switch-harfbuzz switch-libwebp \
        switch-libmodplug switch-libvorbis switch-libogg switch-flac switch-mpg123 switch-opusfile \
    ; do \
        dkp-pacman -Q "$pkg" >/dev/null 2>&1 && continue; \
        dkp-pacman -S --noconfirm --needed "$pkg" 2>/dev/null \
            || dkp-pacman -S --noconfirm "$pkg" 2>/dev/null \
            || true; \
    done

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM
ENV DEVKITA64=/opt/devkitpro/devkitA64
ENV PATH="${DEVKITPRO}/tools/bin:${DEVKITA64}/bin:${PATH}"

WORKDIR /rockbot
