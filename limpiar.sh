#!/bin/bash

echo "Limpiando VideoDownloader..."

# Matar SOLO el ejecutable del bundle de este proyecto.
# IMPORTANTE: no usar `pkill -f VideoDownloader` (patrón demasiado genérico:
# matchea procesos de extensiones de VSCode con `--folder-uri` al
# workspace `LGA_VideoDownloader` y provoca que VSCode los relance varias
# veces al arrancar el script). Apuntar al path completo del ejecutable
# dentro del .app.
pkill -f "VideoDownloader.app/Contents/MacOS/VideoDownloader" 2>/dev/null && echo "   - VideoDownloader terminado" || echo "   - VideoDownloader no estaba en ejecución"
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
