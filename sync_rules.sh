#!/bin/sh
set -eu
# Sincroniza los tres espejos de reglas (AGENTS.md / CLAUDE.md / .cursor).
# La logica esta en tools/sync_rules.py; esto es solo el wrapper de macOS/Linux.
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$SCRIPT_DIR/tools/sync_rules.py"

if [ ! -f "$SCRIPT" ]; then
    echo "ERROR: no se encontro \"$SCRIPT\"." >&2
    exit 1
fi

if [ -x "$SCRIPT_DIR/python_runtime/macos/python3/bin/python3" ]; then
    exec "$SCRIPT_DIR/python_runtime/macos/python3/bin/python3" "$SCRIPT" "$@"
fi
exec python3 "$SCRIPT" "$@"
