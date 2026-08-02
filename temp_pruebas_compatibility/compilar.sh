#!/bin/bash

echo "Compilando VideoDownloader..."

# Matar el proceso VideoDownloader si está en ejecución
pkill -f VideoDownloader || echo "No se encontró el proceso VideoDownloader en ejecución."
sleep 1

# Crear directorio de compilación si no existe
mkdir -p build
cd build

# Configurar el proyecto con la ruta correcta de Qt y configuraciones de compatibilidad
cmake .. -G "Unix Makefiles" \
    -DCMAKE_PREFIX_PATH="$HOME/Qt/6.8.2/macos" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Compilar el proyecto
cmake --build .

# Volver al directorio principal
cd ..

# Copiar dependencias de Qt al bundle usando macdeployqt
echo "Copiando dependencias de Qt al bundle..."
QT_PATH="$HOME/Qt/6.8.2/macos"
if [ -d "$QT_PATH" ]; then
    export PATH="$QT_PATH/bin:$PATH"
    "$QT_PATH/bin/macdeployqt" build/VideoDownloader.app
    echo "Dependencias de Qt copiadas exitosamente."
else
    echo "Advertencia: Qt no encontrado en $QT_PATH. La aplicación puede no ejecutarse correctamente."
fi

# Crear carpeta toolsmac en el bundle y copiar herramientas
echo ""
echo "Preparando carpeta toolsmac..."
if [ -d "toolsmac" ]; then
    echo "Copiando herramientas desde carpeta toolsmac del proyecto..."
    cp -r toolsmac build/VideoDownloader.app/Contents/MacOS/
    echo "Herramientas copiadas exitosamente."
else
    echo "Carpeta toolsmac del proyecto no encontrada o vacía."
fi

echo ""
echo "Compilación completada. Ejecutando VideoDownloader..."
echo ""

# Ejecutar la aplicación desde el bundle
./build/VideoDownloader.app/Contents/MacOS/VideoDownloader
