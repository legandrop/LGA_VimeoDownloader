#!/bin/bash

show_help() {
    echo "Uso: $0 [--no-run] [--wait]"
    echo ""
    echo "Opciones:"
    echo "  --no-run   Compilar sin lanzar la app"
    echo "  --wait     Dejar la app en foreground: la terminal queda retenida hasta"
    echo "             cerrarla y se ven su stdout/stderr y su exit code."
    echo "             Por defecto la app se lanza en background y el script termina."
}

NO_RUN=false
# CONVENCION LGA — por defecto la app se lanza en BACKGROUND y el script termina enseguida.
# Dejarla en foreground retiene la terminal hasta que alguien cierre la app a mano, lo que
# cuelga al que compila (y a cualquier agente) por tiempo indefinido.
# Con --wait se recupera el foreground, util para ver un crash o un exit code.
WAIT_FOR_APP=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-run) NO_RUN=true; shift ;;
        --wait) WAIT_FOR_APP=true; shift ;;
        --help) show_help; exit 0 ;;
        *) echo "Opcion desconocida: $1"; show_help; exit 1 ;;
    esac
done

echo "Compilando VideoDownloader..."

# Matar SOLO el ejecutable del bundle de este proyecto.
# IMPORTANTE: no usar `pkill -f VideoDownloader` (patrón demasiado genérico:
# matchea procesos de extensiones de VSCode con `--folder-uri` al
# workspace `LGA_VideoDownloader` y provoca que VSCode los relance varias
# veces al arrancar el script). Apuntar al path completo del ejecutable
# dentro del .app.
pkill -f "VideoDownloader.app/Contents/MacOS/VideoDownloader" 2>/dev/null && echo "   - VideoDownloader terminado" || echo "   - VideoDownloader no estaba en ejecución"
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
    # "$QT_PATH/bin/macdeployqt" build/VideoDownloader.app -libpath="$QT_PATH/lib"
    echo "Dependencias de Qt disponibles en el sistema (Homebrew)."
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

# Refrescar el cache de iconos del bundle: tras cambiar el .icns, el Dock/Finder pueden
# seguir mostrando el icono viejo por cache (iconservices). touch + lsregister -f fuerzan
# a re-leer el icono. Si el cache de Tahoe sigue pegajoso, cerrar sesion y volver a entrar.
LSREG="/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister"
if [ -d "build/VideoDownloader.app" ]; then
    touch "build/VideoDownloader.app"
    [ -x "$LSREG" ] && "$LSREG" -f "build/VideoDownloader.app" >/dev/null 2>&1 || true
fi

echo ""
echo "Compilación completada. Ejecutando VideoDownloader..."
echo ""

if [ "$NO_RUN" = "true" ]; then
    echo "Ejecucion omitida (--no-run)"
    exit 0
fi

# Ejecutar la aplicación desde el bundle.
# CONVENCION LGA — por defecto en BACKGROUND (ver comentario de WAIT_FOR_APP arriba).
export QT_QPA_PLATFORM_PLUGIN_PATH="/opt/homebrew/share/qt/plugins/platforms"
APP_BIN="./build/VideoDownloader.app/Contents/MacOS/VideoDownloader"
if [ "$WAIT_FOR_APP" = "true" ]; then
    "$APP_BIN"
else
    "$APP_BIN" >/dev/null 2>&1 &
    disown
    echo "   PID $! (background)."
    echo "   Usa --wait si necesitas ver su salida o su exit code en la terminal."
fi
