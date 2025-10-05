#!/bin/bash

# BORRAR DEPLOY ANTERIOR
if [ -d "deploy" ]; then
    echo "Eliminando deploy anterior..."
    rm -rf deploy
fi

echo "Implementando VimeoDownloader..."

# Matar procesos previos si están en ejecución
pkill -f VimeoDownloader || echo "No se encontró el proceso VimeoDownloader en ejecución."
sleep 1

# Verificar que Qt está instalado
QT_PATH="$HOME/Qt/6.8.2/macos"
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Qt 6.8.2 no está instalado en $QT_PATH"
    exit 1
fi

# Añadir Qt al PATH
export PATH="$QT_PATH/bin:$PATH"

# Crear directorio de implementación si no existe
mkdir -p deploy

# Compilar el proyecto en modo Release con configuraciones de compatibilidad
mkdir -p build
cd build

# Configurar con las nuevas opciones de compatibilidad
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
cmake .. \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

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
    <string>1.0.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15.0</string>
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

# Usar macdeployqt para copiar todas las dependencias necesarias
"$QT_PATH/bin/macdeployqt" deploy/VimeoDownloader.app

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
./deploy/VimeoDownloader.app/Contents/MacOS/VimeoDownloader
