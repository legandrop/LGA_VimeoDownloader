#!/bin/sh
# Sincroniza los tres espejos de reglas (AGENTS.md / CLAUDE.md / .cursor).
# La logica esta en tools/sync_rules.py; esto es solo el wrapper de Unix.
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$DIR/tools/sync_rules.py"

if [ ! -f "$SCRIPT" ]; then
    echo "ERROR: No se encontro \"$SCRIPT\"."
    exit 1
fi

# Orden de busqueda del interprete. Cada candidato se PRUEBA corriendolo: en
# Windows (Git Bash), "python" y "python3" existen en el PATH como alias del
# Microsoft Store, imprimen "Python was not found" y salen con error, asi que
# verificar que estan no alcanza. Ver la seccion "Python" de AGENTS.md.
PY=""
for c in "$DIR/python_runtime/windows/python.exe" \
         "$DIR/python_runtime/macos/python3/python3" \
         "$DIR/../LGA_FileManagerS3/python_runtime/windows/python.exe" \
         "$DIR/../LGA_FileManagerS3/python_runtime/macos/python3/python3"; do
    # El binario de macOS es el symlink `python3` que cuelga de
    # python_runtime/macos/python3/, que es la ruta que usa AppPathManager. NO es
    # bin/python3: eso apunta adentro de un `bin/` que el .gitignore de
    # FileManagerS3 ignora, asi que en un clon nuevo no existe.
    #
    # -f ademas de -x porque en un directorio -x significa "se puede entrar" y da
    # true, y el padre `python3` ES un directorio.
    [ -f "$c" ] && [ -x "$c" ] || continue
    if "$c" -c "pass" >/dev/null 2>&1; then PY="$c"; break; fi
done
if [ -z "$PY" ]; then
    for c in "py -3" python3 python; do
        if $c -c "pass" >/dev/null 2>&1; then PY="$c"; break; fi
    done
fi

if [ -z "$PY" ]; then
    echo "ERROR: no encontre un interprete de Python usable."
    exit 1
fi

exec $PY "$SCRIPT" "$@"
