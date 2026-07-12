#!/bin/bash

echo "Compilando VimeoDownloader..."

# Matar el proceso VimeoDownloader si está en ejecución
pkill -f VimeoDownloader || echo "No se encontró el proceso VimeoDownloader en ejecución."
sleep 1

# Crear directorio de compilación si no existe
mkdir -p build
cd build

# Configurar el proyecto con Qt de Homebrew (compatible con macOS Tahoe)
# Usar solo arquitectura ARM64 ya que Qt de Homebrew solo soporta ARM64
export CMAKE_PREFIX_PATH="/opt/homebrew"
export Qt6_DIR="/opt/homebrew/lib/cmake/Qt6"
SDK_PATH="$(xcrun --sdk macosx --show-sdk-path)"
if [ -d "$SDK_PATH" ]; then
    export SDKROOT="$SDK_PATH"
else
    echo "Advertencia: SDK de macOS no encontrado vía xcrun."
fi

cmake .. -G "Unix Makefiles" \
    -DCMAKE_PREFIX_PATH="/opt/homebrew" \
    -DQt6_DIR="/opt/homebrew/lib/cmake/Qt6" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_OSX_ARCHITECTURES="arm64" \
    -DCMAKE_OSX_SYSROOT="$SDK_PATH"

# Compilar el proyecto
cmake --build .

# Volver al directorio principal
cd ..

# Copiar dependencias de Qt al bundle usando macdeployqt
echo "Copiando dependencias de Qt al bundle..."
QT_PATH="/opt/homebrew"
if [ -d "$QT_PATH" ]; then
    export PATH="$QT_PATH/bin:$PATH"
    export DYLD_LIBRARY_PATH="$QT_PATH/lib:$DYLD_LIBRARY_PATH"
    # Nota: macdeployqt tiene problemas con Homebrew Qt, pero la app funciona sin él
    # "$QT_PATH/bin/macdeployqt" build/VimeoDownloader.app -libpath="$QT_PATH/lib"
    echo "Dependencias de Qt disponibles en el sistema (Homebrew)."
else
    echo "Advertencia: Qt no encontrado en $QT_PATH. La aplicación puede no ejecutarse correctamente."
fi

# Crear carpeta toolsmac en el bundle y copiar herramientas
echo ""
echo "Preparando carpeta toolsmac..."
if [ -d "toolsmac" ]; then
    echo "Copiando herramientas desde carpeta toolsmac del proyecto..."
    cp -r toolsmac build/VimeoDownloader.app/Contents/MacOS/
    echo "Herramientas copiadas exitosamente."
else
    echo "Carpeta toolsmac del proyecto no encontrada o vacía."
fi

# Refrescar el cache de iconos del bundle: tras cambiar el .icns, el Dock/Finder pueden
# seguir mostrando el icono viejo por cache (iconservices). touch + lsregister -f fuerzan
# a re-leer el icono. Si el cache de Tahoe sigue pegajoso, cerrar sesion y volver a entrar.
LSREG="/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister"
if [ -d "build/VimeoDownloader.app" ]; then
    touch "build/VimeoDownloader.app"
    [ -x "$LSREG" ] && "$LSREG" -f "build/VimeoDownloader.app" >/dev/null 2>&1 || true
fi

echo ""
echo "Compilación completada. Ejecutando VimeoDownloader..."
echo ""

# Ejecutar la aplicación desde el bundle
export QT_QPA_PLATFORM_PLUGIN_PATH="/opt/homebrew/share/qt/plugins/platforms"
./build/VimeoDownloader.app/Contents/MacOS/VimeoDownloader
