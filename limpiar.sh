#!/bin/bash

echo "Limpiando VimeoDownloader..."

# Matar SOLO el ejecutable del bundle de este proyecto.
# IMPORTANTE: no usar `pkill -f VimeoDownloader` (patrón demasiado genérico:
# matchea procesos de extensiones de VSCode con `--folder-uri` al
# workspace `LGA_VimeoDownloader` y provoca que VSCode los relance varias
# veces al arrancar el script). Apuntar al path completo del ejecutable
# dentro del .app.
pkill -f "VimeoDownloader.app/Contents/MacOS/VimeoDownloader" 2>/dev/null && echo "   - VimeoDownloader terminado" || echo "   - VimeoDownloader no estaba en ejecución"
sleep 1

# Eliminar directorios de compilación
if [ -d "build" ]; then
    echo "Eliminando directorio build..."
    rm -rf build
fi

if [ -d "deploy" ]; then
    echo "Eliminando directorio deploy..."
    rm -rf deploy
fi

echo "Limpieza completada."
