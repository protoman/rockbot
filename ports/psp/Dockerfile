FROM pspdev/pspdev:latest

RUN apk add --no-cache \
        rsync \
        zip \
        ca-certificates

# Bake SDL2 + deps into the image (tried once at build time, not on every game build).
# Package names vary slightly across pspdev releases; missing ones are skipped.
RUN psp-pacman -Sy --noconfirm || true \
 && for pkg in \
        sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx \
        sdl2-image sdl2-mixer sdl2-ttf sdl2-gfx \
        libpng libjpeg-turbo jpeg freetype2 bzip2 libbz2 \
        harfbuzz harfbuzz-icu libvorbis libogg flac mpg123 opusfile \
        libmodplug modplug libxmp mikmod \
    ; do \
        psp-pacman -Q "$pkg" >/dev/null 2>&1 && continue; \
        psp-pacman -S --noconfirm --needed "$pkg" 2>/dev/null \
            || psp-pacman -S --noconfirm "$pkg" 2>/dev/null \
            || true; \
    done

WORKDIR /rockbot
