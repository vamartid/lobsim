#!/usr/bin/env bash
# Usage: ./detect_deps.sh <source_file>
# Outputs modules this file depends on, recursively

SOURCE_FILE="$1"
INCLUDE_DIR="include"
MODULES=("core" "engine" "simulator" "utils")
declare -A SEEN

function scan_file() {
    local file="$1"

    # Avoid reprocessing the same file
    [[ -z "$file" || -n "${SEEN[$file]}" ]] && return
    SEEN[$file]=1

    # Read all #include lines
    grep -E '^#include\s+"([^"]+)"' "$file" | while read -r line; do
        local inc=$(echo "$line" | sed -E 's/^#include\s+"([^"]+)"/\1/')
        local inc_path="$INCLUDE_DIR/$inc"

        # Determine which module this include belongs to
        for mod in "${MODULES[@]}"; do
            if [[ "$inc" == "$mod/"* ]]; then
                echo "$mod"
            fi
        done

        # Recurse into included header if it exists
        [[ -f "$inc_path" ]] && scan_file "$inc_path"
    done
}

scan_file "$SOURCE_FILE" | sort -u
