#!/bin/bash
# Usage: ./build-psp.sh <1|2> [--no-package]
# CMake build with the pspdev SDK; see ports/psp/build.sh (PSPDEV selects the SDK).
exec "$(cd "$(dirname "$0")" && pwd)/ports/psp/build.sh" "$@"
