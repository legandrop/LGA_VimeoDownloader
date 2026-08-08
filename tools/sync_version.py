#!/usr/bin/env python3
"""Deriva el archivo VERSION desde CMakeLists.txt.

**`CMakeLists.txt` es la UNICA fuente de verdad** del numero de version, via
`project(VideoDownloader VERSION x.y ...)`. El archivo `VERSION` es un espejo DERIVADO que
existe para que los scripts de shell (deploy, DMG, instalador) lean una linea en vez de
parsear CMake, y para que este repo tenga la misma forma que el resto de las apps LGA.

Este script NO decide versiones ni las sube: solo copia. El bump de `project(...)` lo hace
el usuario cuando corresponde, y el ChangeLog puede ir por delante acumulando entradas
dentro de la version siguiente (ver las reglas del repo).

    python3 tools/sync_version.py               # escribe VERSION con lo que dice CMakeLists
    python3 tools/sync_version.py --check-only  # falla si estan desincronizados
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[1]
CMAKE_FILE = ROOT_DIR / "CMakeLists.txt"
VERSION_FILE = ROOT_DIR / "VERSION"


def write_lf(path: Path, content: str) -> None:
    """Escribe siempre con LF.

    El modo texto de Python traduce '\\n' al separador del sistema, asi que en Windows
    `write_text` dejaria CRLF contra un `.gitattributes` que pide LF. Git lo normaliza al
    indexar, con lo cual el repo nunca queda mal, pero la copia en disco si: aparece el
    warning de conversion en cada commit.
    """
    with path.open("w", encoding="utf-8", newline="\n") as fh:
        fh.write(content)


def cmake_version() -> str:
    content = CMAKE_FILE.read_text(encoding="utf-8")
    match = re.search(r"project\(\s*VideoDownloader\s+VERSION\s+([0-9]+(?:\.[0-9]+)+)", content)
    if not match:
        raise ValueError("No se encontro project(VideoDownloader VERSION ...) en CMakeLists.txt")
    return match.group(1)


def main() -> int:
    check_only = "--check-only" in sys.argv[1:]
    resolved = cmake_version()
    current = VERSION_FILE.read_text(encoding="utf-8").strip() if VERSION_FILE.exists() else None

    if check_only:
        if current != resolved:
            print(f"[sync_version] ERROR: VERSION dice {current!r} y CMakeLists.txt dice {resolved!r}",
                  file=sys.stderr)
            print("[sync_version] Corre ./sync_version.sh para sincronizarlo.", file=sys.stderr)
            return 1
        print(f"[sync_version] OK: {resolved} en CMakeLists.txt y VERSION")
        return 0

    write_lf(VERSION_FILE, resolved + "\n")
    print(f"[sync_version] VERSION: {current} -> {resolved} (fuente: CMakeLists.txt)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # pylint: disable=broad-except
        print(f"[sync_version] ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
