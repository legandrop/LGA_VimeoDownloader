#!/bin/bash

echo "Limpiando VimeoDownloader..."

# Matar el proceso VimeoDownloader si está en ejecución
pkill -f VimeoDownloader || echo "No se encontró el proceso VimeoDownloader en ejecución."
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
