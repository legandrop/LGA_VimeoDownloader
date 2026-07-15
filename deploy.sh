#!/bin/bash

# BORRAR DEPLOY ANTERIOR
if [ -d "deploy" ]; then
    echo "Eliminando deploy anterior..."
    rm -rf deploy
fi

echo "Implementando VimeoDownloader..."

# Matar procesos previos si están en ejecución.
# IMPORTANTE: no usar `pkill -f VimeoDownloader` (patrón demasiado genérico:
# matchea procesos de extensiones de VSCode con `--folder-uri` al
# workspace `LGA_VimeoDownloader` y provoca que VSCode los relance varias
# veces al arrancar el script). Apuntar al path completo del ejecutable
# dentro del .app.
pkill -f "VimeoDownloader.app/Contents/MacOS/VimeoDownloader" 2>/dev/null && echo "   - VimeoDownloader terminado" || echo "   - VimeoDownloader no estaba en ejecución"
sleep 1

# Verificar que Qt de Homebrew está instalado (compatible con macOS Tahoe)
QT_PATH="/opt/homebrew"
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Qt no está instalado en $QT_PATH (Homebrew)"
    exit 1
fi

# Configurar variables de entorno para Qt de Homebrew
export CMAKE_PREFIX_PATH="$QT_PATH"
export Qt6_DIR="$QT_PATH/lib/cmake/Qt6"
export PATH="$QT_PATH/bin:$PATH"
SDK_PATH="$(xcrun --sdk macosx --show-sdk-path)"
if [ -d "$SDK_PATH" ]; then
    export SDKROOT="$SDK_PATH"
else
    echo "Advertencia: SDK de macOS no encontrado vía xcrun."
fi

# Crear directorio de implementación si no existe
mkdir -p deploy

# Compilar el proyecto en modo Release con configuraciones de compatibilidad
mkdir -p build
cd build

# Configurar el proyecto con Qt de Homebrew (compatible con macOS Tahoe)
# Usar solo arquitectura ARM64 ya que Qt de Homebrew solo soporta ARM64
cmake .. -G "Unix Makefiles" \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DQt6_DIR="$QT_PATH/lib/cmake/Qt6" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_OSX_ARCHITECTURES="arm64" \
    -DCMAKE_OSX_SYSROOT="$SDK_PATH"

cmake --build . --config Release
cd ..

# Crear estructura del bundle
mkdir -p deploy/VimeoDownloader.app/Contents/{MacOS,Resources,Frameworks}
cp build/VimeoDownloader.app/Contents/MacOS/VimeoDownloader deploy/VimeoDownloader.app/Contents/MacOS/

# Copiar el ícono al bundle si existe
if [ -f "resources/icons/LGA_VimeoDownloader.icns" ]; then
    echo "Copiando ícono al bundle..."
    cp resources/icons/LGA_VimeoDownloader.icns deploy/VimeoDownloader.app/Contents/Resources/
fi

# Crear Info.plist con configuración mejorada de compatibilidad
cat > deploy/VimeoDownloader.app/Contents/Info.plist << EOL
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>VimeoDownloader</string>
    <key>CFBundleIconFile</key>
    <string>LGA_VimeoDownloader</string>
    <key>CFBundleIdentifier</key>
    <string>com.lga.vimeodownloader</string>
    <key>CFBundleName</key>
    <string>VimeoDownloader</string>
    <key>CFBundleDisplayName</key>
    <string>Vimeo Downloader</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleVersion</key>
    <string>0.86</string>
    <key>CFBundleShortVersionString</key>
    <string>0.86</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>14.0.0</string>
    <key>LSArchitecturePriority</key>
    <array>
        <string>arm64</string>
        <string>x86_64</string>
    </array>
    <key>LSRequiresNativeExecution</key>
    <false/>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>LSApplicationCategoryType</key>
    <string>public.app-category.utilities</string>
    <key>NSAppTransportSecurity</key>
    <dict>
        <key>NSAllowsArbitraryLoads</key>
        <true/>
    </dict>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
    <key>NSSupportsAutomaticGraphicsSwitching</key>
    <true/>
    <key>NSHumanReadableCopyright</key>
    <string>© 2024 LGA. Todos los derechos reservados.</string>
</dict>
</plist>
EOL

# Nota: macdeployqt tiene problemas con Homebrew Qt en macOS Tahoe
# La aplicación funcionará sin él si Qt está disponible en el sistema destino
# "$QT_PATH/bin/macdeployqt" deploy/VimeoDownloader.app
echo "Omitiendo macdeployqt (problemas conocidos con Homebrew Qt en macOS Tahoe)"

# Crear carpeta toolsmac en deploy y copiar herramientas
echo ""
echo "Preparando carpeta toolsmac para deploy..."
if [ -d "toolsmac" ]; then
    echo "Copiando herramientas a carpeta deploy..."
    cp -r toolsmac deploy/VimeoDownloader.app/Contents/MacOS/
    echo "Herramientas copiadas exitosamente."
else
    echo "Carpeta toolsmac no encontrada o vacía."
fi

# Hacer ejecutable el script
chmod +x deploy/VimeoDownloader.app/Contents/MacOS/VimeoDownloader

echo
echo "Implementación completada. La aplicación portable está en la carpeta 'deploy/VimeoDownloader.app'."
echo

# Ejecutar la aplicación implementada
echo "Ejecutando VimeoDownloader..."
export QT_QPA_PLATFORM_PLUGIN_PATH="/opt/homebrew/share/qt/plugins/platforms"
./deploy/VimeoDownloader.app/Contents/MacOS/VimeoDownloader
