#!/usr/bin/env bash
# Regenerates overlay ports from the vcpkg submodule + stored patches.
# Run this after bumping the vcpkg submodule to pick up upstream changes
# while reapplying local overrides.
#
# Usage: ./ports/update-overlay.sh [port-name]
#   With no argument, updates all overlay ports.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VCPKG_PORTS="${SCRIPT_DIR}/../vcpkg/ports"
OVERLAY_PORTS="${SCRIPT_DIR}"

update_port() {
    local port="$1"
    local upstream="${VCPKG_PORTS}/${port}"
    local overlay="${OVERLAY_PORTS}/${port}"

    if [[ ! -d "${upstream}" ]]; then
        echo "ERROR: upstream port '${port}' not found at ${upstream}" >&2
        return 1
    fi

    echo "Updating overlay port: ${port}"

    # Collect patch files before overwriting
    local patches=("${overlay}"/*.patch)

    # Sync all files from upstream
    cp "${upstream}"/* "${overlay}/"

    # Re-apply patches
    for patch in "${patches[@]}"; do
        [[ -f "${patch}" ]] || continue
        local target="${overlay}/$(basename "${patch}" .patch)"
        if [[ -f "${target}" ]]; then
            echo "  Applying $(basename "${patch}") ..."
            patch --no-backup-if-mismatch "${target}" < "${patch}"
        else
            echo "  WARNING: patch target '${target}' not found, skipping" >&2
        fi
    done

    # Regenerate the patch files to reflect any upstream context changes
    for patch in "${patches[@]}"; do
        [[ -f "${patch}" ]] || continue
        local filename="$(basename "${patch}" .patch)"
        diff -u "${upstream}/${filename}" "${overlay}/${filename}" > "${patch}" || true
    done

    echo "  Done."
}

if [[ $# -gt 0 ]]; then
    update_port "$1"
else
    for overlay_dir in "${OVERLAY_PORTS}"/*/; do
        [[ -d "${overlay_dir}" ]] || continue
        update_port "$(basename "${overlay_dir}")"
    done
fi
