#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
sync_rules.py
=============

Mantiene sincronizados los TRES espejos de reglas del repo:

    AGENTS.md                       (Codex)
    CLAUDE.md                       (Claude Code)
    .cursor/rules/instructions.mdc  (Cursor)

Los tres tienen el MISMO contenido. Lo unico que difiere es el frontmatter YAML
del `.mdc` (`alwaysApply: true`), que se agrega y se saca solo.

Uso
---
    python3 tools/sync_rules.py              # sincroniza desde el mas reciente
    python3 tools/sync_rules.py --check      # no escribe; sale 1 si difieren
    python3 tools/sync_rules.py --from AGENTS.md
    python3 tools/sync_rules.py --install-hook

Por que existe: la regla de "sincronizar los tres en la misma pasada" es manual y
se olvida. Ya paso: un `.mdc` quedo diez dias sin recibir cambios que si estaban
en el `AGENTS.md`, y nadie se dio cuenta hasta que alguien comparo a mano.

Compatible con Python 3.6+ (el `python3` del sistema en macOS es 3.9, y en
Windows puede ser el embebido del repo).
"""
from __future__ import annotations

import argparse
import os
import sys

FRONTMATTER = "---\nalwaysApply: true\n---\n\n"

AGENTS = "AGENTS.md"
CLAUDE = "CLAUDE.md"
CURSOR = os.path.join(".cursor", "rules", "instructions.mdc")

HOOK = """#!/bin/sh
# Hook de LGA: corta el commit si los tres espejos de reglas no coinciden.
# Verifica, NO reescribe: lo que un pre-commit modifica no entra al commit salvo
# que haga `git add` por dentro, y eso sorprende a cualquiera.
# Instalado por `tools/sync_rules.py --install-hook` (via core.hooksPath).
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
[ -f "$SCRIPT_DIR/tools/sync_rules.py" ] || exit 0
if command -v python3 >/dev/null 2>&1; then
    PY=python3
elif command -v py >/dev/null 2>&1; then
    PY="py -3"
else
    PY=python
fi
$PY "$SCRIPT_DIR/tools/sync_rules.py" --check --quiet-ok || {
    echo ""
    echo "  Corregilo con:  ./sync_rules.sh      (Windows: sync_rules.bat)"
    echo "  Para saltear:   git commit --no-verify"
    exit 1
}
"""


def repo_root(start):
    """Sube hasta encontrar el .git. Permite correr el script desde cualquier lado."""
    d = os.path.abspath(start)
    while True:
        if os.path.isdir(os.path.join(d, ".git")) or os.path.isfile(os.path.join(d, ".git")):
            return d
        parent = os.path.dirname(d)
        if parent == d:
            return os.path.abspath(start)
        d = parent


def read(path):
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def write(path, content):
    """Escribe preservando el CRLF si el archivo ya lo tenia.

    `Path.write_text(newline=...)` recien existe en Python 3.10; `open()` lo
    acepta en todas las versiones.
    """
    newline = "\n"
    if os.path.exists(path):
        with open(path, "rb") as fh:
            if b"\r\n" in fh.read():
                newline = "\r\n"
    os.makedirs(os.path.dirname(os.path.abspath(path)) or ".", exist_ok=True)
    with open(path, "w", encoding="utf-8", newline=newline) as fh:
        fh.write(content)


def body_of(path):
    """Contenido sin el frontmatter del .mdc, normalizado a LF."""
    text = read(path).replace("\r\n", "\n")
    if path.endswith(".mdc") and text.startswith("---"):
        end = text.find("\n---", 3)
        if end != -1:
            text = text[end + 4:].lstrip("\n")
    return text


def present(root):
    return [p for p in (AGENTS, CLAUDE, CURSOR) if os.path.isfile(os.path.join(root, p))]


def main():
    ap = argparse.ArgumentParser(description="Sincroniza los tres espejos de reglas.")
    ap.add_argument("--check", action="store_true",
                    help="No escribe nada; sale 1 si los espejos difieren.")
    ap.add_argument("--from", dest="source", default=None,
                    help="Archivo fuente. Default: el modificado mas recientemente.")
    ap.add_argument("--install-hook", action="store_true",
                    help="Instala el hook pre-commit (core.hooksPath = .githooks).")
    ap.add_argument("--quiet-ok", action="store_true",
                    help="No imprimir nada cuando ya estan sincronizados.")
    args = ap.parse_args()

    root = repo_root(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root)

    if args.install_hook:
        return install_hook(root)

    files = present(root)
    if not files:
        print("[sync_rules] este repo no tiene archivos de reglas; nada que hacer.")
        return 0

    bodies = {p: body_of(p) for p in files}
    distinct = set(bodies.values())

    if len(distinct) == 1 and len(files) == 3:
        if not args.quiet_ok:
            print("[sync_rules] OK: los 3 espejos coinciden.")
        return 0

    if args.check:
        print("[sync_rules] ERROR: los espejos de reglas NO coinciden.")
        for p in (AGENTS, CLAUDE, CURSOR):
            if p not in files:
                print("    FALTA      %s" % p)
        if len(distinct) > 1:
            for p in files:
                print("    %-32s %d bytes" % (p, len(bodies[p])))
        return 1

    if args.source:
        src = args.source
        if not os.path.isfile(src):
            print("[sync_rules] ERROR: no existe la fuente %s" % src)
            return 1
    else:
        src = max(files, key=lambda p: os.path.getmtime(p))

    body = body_of(src)
    print("[sync_rules] fuente: %s" % src)
    for target in (AGENTS, CLAUDE, CURSOR):
        content = FRONTMATTER + body if target == CURSOR else body
        if os.path.isfile(target) and read(target).replace("\r\n", "\n") == content:
            continue
        write(target, content)
        print("[sync_rules]   -> %s" % target)
    print("[sync_rules] OK: los 3 espejos quedaron iguales.")
    return 0


def install_hook(root):
    hooks_dir = os.path.join(root, ".githooks")
    os.makedirs(hooks_dir, exist_ok=True)
    hook_path = os.path.join(hooks_dir, "pre-commit")
    write(hook_path, HOOK)
    os.chmod(hook_path, 0o755)

    import subprocess
    subprocess.run(["git", "config", "core.hooksPath", ".githooks"], cwd=root, check=False)
    current = subprocess.run(["git", "config", "core.hooksPath"], cwd=root,
                             capture_output=True, text=True).stdout.strip()
    print("[sync_rules] hook instalado en .githooks/pre-commit")
    print("[sync_rules] core.hooksPath = %s" % (current or "(no configurado)"))
    if current != ".githooks":
        print("[sync_rules] AVISO: no se pudo configurar core.hooksPath.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
