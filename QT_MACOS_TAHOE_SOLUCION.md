# Diagnóstico y solución tras actualizar a macOS Tahoe

Este documento resume el problema que apareció al actualizar a macOS **Tahoe** y cómo se resolvió para que `compilar.sh` vuelva a funcionar en proyectos Qt/C++.

## Qué fallaba

- Después de la actualización, `compilar.sh` detenía la compilación con errores de *linker* como:
  - `ld: warning: ignoring file '.../QtWidgets.framework' found architecture 'arm64', required architecture 'x86_64'`
  - `Undefined symbols for architecture x86_64`
- Incluso cuando el build llegaba al final, la ejecución fallaba con:
  - `qt.qpa.plugin: Could not find the Qt platform plugin "cocoa" in ""`
  - `This application failed to start because no Qt platform plugin could be initialized`

## Causa raíz

1. **Dos instalaciones de Qt activas**:
   - `/Users/<usuario>/Qt/6.8.2/macos` (binario universal x86_64/arm64, pero incompatible con Tahoe: exige instrucciones *neon crc32* y falla al ejecutar `uic`).
   - `/opt/homebrew` (Qt instalado por Homebrew, sólo arm64 y compatible con Tahoe).
2. El script intentaba generar un binario universal (`x86_64;arm64`), forzando al *linker* a buscar librerías x86_64 inexistentes en la instalación de Homebrew.
3. Al usar Homebrew Qt, `macdeployqt` no encontraba los plugins de plataforma; la aplicación quedaba sin `libqcocoa.dylib`.

## Solución aplicada

1. **Forzar CMake a usar Qt de Homebrew**:
   ```sh
   export CMAKE_PREFIX_PATH="/opt/homebrew"
   export Qt6_DIR="/opt/homebrew/lib/cmake/Qt6"
   ```
2. **Compilar sólo para arm64** (macOS Tahoe en Apple Silicon):
   ```sh
   -DCMAKE_OSX_ARCHITECTURES="arm64"
   ```
3. **Alinear el target del sistema operativo** con las librerías de Homebrew Qt (construidas para macOS 14):
   ```sh
   -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
   ```
4. **Evitar el `macdeployqt` de Homebrew** (que falla en el sandbox) y usar Qt en tiempo de ejecución exportando la ruta del plugin cocoa justo antes de lanzar la app:
   ```sh
   export QT_QPA_PLATFORM_PLUGIN_PATH="/opt/homebrew/share/qt/plugins/platforms"
   ```
5. (Opcional) Si se necesita empaquetar un `.app` auto-contenido, ejecutar `macdeployqt` fuera del sandbox o usar la instalación oficial de Qt ya parchada para Tahoe.

## Resultado

- `compilar.sh` vuelve a generar y enlazar el proyecto sin errores.
- La aplicación arranca correctamente en macOS Tahoe cuando se ejecuta en el entorno gráfico normal (los mensajes `PasteBoard...` sólo aparecen en entornos sin servicios gráficos, como el sandbox).
- La misma receta se puede replicar en otros proyectos Qt/C++ afectados por la actualización.

## Pasos para reutilizar en otros proyectos

1. Edita `compilar.sh` (o tu script equivalente) y apunta `CMAKE_PREFIX_PATH` y `Qt6_DIR` hacia `/opt/homebrew`.
2. Ajusta las banderas de CMake para que sólo generen binarios `arm64` y establezcan `CMAKE_OSX_DEPLOYMENT_TARGET` a `14.0` (o la versión que reporta Homebrew `otool -l QtCore.framework/QtCore | grep -A1 LC_VERSION_MIN_MACOSX`).
3. Antes de lanzar la aplicación desde el script, exporta `QT_QPA_PLATFORM_PLUGIN_PATH`:
   ```sh
   export QT_QPA_PLATFORM_PLUGIN_PATH="/opt/homebrew/share/qt/plugins/platforms"
   ```
4. Si tus apps usan `macdeployqt`, ejecútalo manualmente fuera del script y verifica con `otool -L` que todos los frameworks estén empacados.

Con estos ajustes tus proyectos Qt deberían compilar y ejecutarse normalmente en macOS Tahoe sin necesidad de reinstalar el SDK de Qt oficial.
