#!/usr/bin/env python3
"""
Completa lo que macdeployqt deja afuera con el Qt de Homebrew.

macdeployqt copia los frameworks que encuentra, pero no persigue las dependencias que los
kegs de Homebrew arrastran entre si (QtGui -> QtDBus -> libdbus-1). El sintoma es un
`dyld: Library not loaded` recien al ejecutar el bundle en otra maquina.

Este script recorre el bundle en CICLO: por cada Mach-O busca dependencias que sigan
apuntando a /opt/homebrew (o a cualquier prefijo no-sistema), copia lo que falte adentro de
Contents/Frameworks y reescribe la referencia a @rpath. Repite hasta que no aparece nada
nuevo, porque cada cosa que copia puede traer sus propias dependencias.

Uso: bundle_fixup.py <ruta al .app>
"""

import glob
import os
import shutil
import subprocess
import sys

# Prefijos que SI van adentro del bundle. Todo lo que este afuera de estos (o sea
# /usr/lib y /System) es del sistema y no se toca.
EXTERNAL_PREFIXES = ("/opt/homebrew", "/usr/local")

MACHO_MAGIC = (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xca\xfe\xba\xbe")

# Donde buscar lo que macdeployqt dejo declarado como @rpath pero no copio. Homebrew parte
# Qt en un keg por modulo, asi que no alcanza con un solo directorio.
SEARCH_ROOTS = sorted(glob.glob("/opt/homebrew/opt/*/lib")) + ["/opt/homebrew/lib"]


def find_in_search_roots(rel):
    """Resuelve "QtSvg.framework/Versions/A/QtSvg" o "libbrotlicommon.1.dylib" en los kegs."""
    for root in SEARCH_ROOTS:
        candidate = os.path.join(root, rel)
        if os.path.exists(candidate):
            return candidate
    return None


def is_macho(path):
    if os.path.islink(path) or not os.path.isfile(path):
        return False
    try:
        with open(path, "rb") as fh:
            return fh.read(4) in MACHO_MAGIC
    except OSError:
        return False


def macho_files(root):
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            path = os.path.join(dirpath, name)
            if is_macho(path):
                yield path


def otool_lines(path):
    out = subprocess.run(["otool", "-L", path], capture_output=True, text=True).stdout
    return [line.strip().split(" (")[0] for line in out.splitlines()[1:]]


def install_id(path):
    """
    El id (LC_ID_DYLIB) sale de `otool -D`, NO de la primera linea de `otool -L`.
    Parece lo mismo y no lo es: hay plugins cuyo primer renglon de -L es una dependencia
    real, asi que asumir la posicion se come esa dependencia sin avisar.
    """
    out = subprocess.run(["otool", "-D", path], capture_output=True, text=True).stdout
    lines = [l.strip() for l in out.splitlines()[1:] if l.strip()]
    return lines[0] if lines else None


def dependencies(path):
    """Dependencias reales: todo lo de `otool -L` menos el propio id."""
    own = install_id(path)
    return [d for d in otool_lines(path) if d != own]


def is_external(dep):
    return any(dep.startswith(prefix) for prefix in EXTERNAL_PREFIXES)


def framework_rpath(dep):
    """/opt/homebrew/.../QtDBus.framework/Versions/A/QtDBus -> QtDBus.framework/Versions/A/QtDBus"""
    marker = ".framework/"
    idx = dep.find(marker)
    if idx == -1:
        return os.path.basename(dep)
    start = dep.rfind("/", 0, idx) + 1
    return dep[start:]


def framework_source_root(dep):
    """Carpeta .framework de origen para copiar entera."""
    marker = ".framework"
    idx = dep.find(marker)
    if idx == -1:
        return None
    return dep[: idx + len(marker)]


def main():
    if len(sys.argv) != 2:
        print("uso: bundle_fixup.py <ruta al .app>", file=sys.stderr)
        return 2
    app = os.path.abspath(sys.argv[1].rstrip("/"))
    frameworks = os.path.join(app, "Contents", "Frameworks")
    os.makedirs(frameworks, exist_ok=True)

    copied, rewritten = [], 0

    for _ in range(10):  # cota: en la practica converge en 2 o 3 vueltas
        pending = False
        for macho in list(macho_files(app)):
            own_id = install_id(macho)
            deps = dependencies(macho)

            # El id propio tambien se normaliza: si queda apuntando al keg, cualquiera que
            # linkee por ese path despues falla.
            if own_id and is_external(own_id):
                rel = framework_rpath(own_id)
                subprocess.run(["install_name_tool", "-id", "@rpath/" + rel, macho],
                               capture_output=True)
                rewritten += 1
                pending = True

            for dep in deps:
                if not is_external(dep):
                    continue
                rel = framework_rpath(dep)
                target = os.path.join(frameworks, rel)

                if not os.path.exists(target):
                    src_fw = framework_source_root(dep)
                    if src_fw and os.path.isdir(src_fw):
                        dest_fw = os.path.join(frameworks, os.path.basename(src_fw))
                        if not os.path.exists(dest_fw):
                            subprocess.run(["ditto", src_fw, dest_fw], check=True)
                            copied.append(os.path.basename(src_fw))
                    elif os.path.isfile(dep):
                        os.makedirs(os.path.dirname(target), exist_ok=True)
                        shutil.copy2(dep, target)
                        copied.append(os.path.basename(dep))

                subprocess.run(["install_name_tool", "-change", dep, "@rpath/" + rel, macho],
                               capture_output=True)
                rewritten += 1
                pending = True

        # Segundo caso: macdeployqt ya reescribio la referencia a @rpath pero NUNCA copio el
        # archivo. No hay nada que reescribir, falta traerlo.
        for macho in list(macho_files(app)):
            for dep in dependencies(macho):
                if not dep.startswith("@rpath/"):
                    continue
                rel = dep[len("@rpath/"):]
                if os.path.exists(os.path.join(frameworks, rel)):
                    continue
                source = find_in_search_roots(rel)
                if not source:
                    continue
                src_fw = framework_source_root(source)
                if src_fw and os.path.isdir(src_fw):
                    dest = os.path.join(frameworks, os.path.basename(src_fw))
                    if not os.path.exists(dest):
                        subprocess.run(["ditto", src_fw, dest], check=True)
                        copied.append(os.path.basename(src_fw))
                        pending = True
                elif os.path.isfile(source):
                    dest = os.path.join(frameworks, rel)
                    os.makedirs(os.path.dirname(dest), exist_ok=True)
                    shutil.copy2(source, dest)
                    copied.append(os.path.basename(source))
                    pending = True

        if not pending:
            break

    # Verificacion: nada puede quedar apuntando afuera, y todo @rpath tiene que existir.
    leftovers, missing = [], []
    for macho in macho_files(app):
        for dep in dependencies(macho):
            if is_external(dep):
                leftovers.append((macho, dep))
            elif dep.startswith("@rpath/"):
                if not os.path.exists(os.path.join(frameworks, dep[len("@rpath/"):])):
                    missing.append((macho, dep))

    print(f"[bundle_fixup] copiado al bundle: {sorted(set(copied)) or 'nada'}")
    print(f"[bundle_fixup] referencias reescritas: {rewritten}")
    print(f"[bundle_fixup] referencias externas restantes: {len(leftovers)}")
    for m, d in leftovers[:10]:
        print(f"   {m.replace(app + '/', '')} -> {d}")
    print(f"[bundle_fixup] dependencias @rpath faltantes: {len(missing)}")
    for m, d in missing[:10]:
        print(f"   {m.replace(app + '/', '')} -> {d}")

    return 1 if (leftovers or missing) else 0


if __name__ == "__main__":
    sys.exit(main())
