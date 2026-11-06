#!/usr/bin/env bash
# Writes the version from version_name.txt into file/version.h (no-op if equal).
# Called by the build scripts; can also be run by hand.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
sync_version_header
