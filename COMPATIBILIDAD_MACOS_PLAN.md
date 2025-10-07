# Plan de Compatibilidad para VimeoDownloader en macOS

## Objetivo General
Hacer que la aplicación VimeoDownloader sea compatible con versiones antiguas y nuevas de macOS mediante compilación universal (soporte para arquitecturas x86_64 -Intel- y arm64 -Apple Silicon-) y pruebas en múltiples entornos. Esto se basa en la combinación de la Opción 1 (Compilación Universal) y la Opción 4 (Pruebas) de nuestro análisis inicial.

## Contexto del Proyecto
- **Tecnologías usadas**: C++ con Qt6, compilado vía CMake, con scripts `compilar.sh` y `deploy.sh` para build y despliegue.
- **Estado actual**: La app compila y despliega bien en el entorno actual (macOS Sonoma con Apple Silicon), pero no soporta arquitecturas Intel (x86_64) ni versiones mínimas específicas de macOS.
- **Dependencias clave**: Qt6, FFmpeg, yt-dlp, y recursos empaquetados en un bundle .app.
- **Arquitecturas actuales**: Solo arm64 (debido a defaults del sistema).

## Cambios Implementados
### 1. Modificaciones en `CMakeLists.txt`
- Agregamos `set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")` para generar binarios universales que funcionen en Macs Intel y Apple Silicon.
- Agregamos `set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")` para establecer macOS 10.13 (High Sierra) como versión mínima soportada, permitiendo compatibilidad hacia atrás.
- Estos cambios son aditivos y no rompen la funcionalidad existente.

### 2. Modificaciones en `compilar.sh`
- Modificamos la línea de `cmake` para incluir los flags de arquitecturas y deployment target.
- Ahora el script soporta cross-compilación para x86_64 desde una máquina arm64.

### 3. Pruebas
- Usamos una carpeta temporal `temp_pruebas_compatibility` para aislar pruebas.
- Verificamos arquitecturas con `lipo`, compilamos, y ejecutamos pruebas básicas.

## Pasos de Ejecución
1. **Compilación Universal**: Ejecutar `compilar.sh` modificado para generar binarios universales.
2. **Verificación**: Usar `lipo` para confirmar arquitecturas (x86_64 y arm64).
3. **Pruebas Básicas**: Ejecutar la app en el entorno actual y verificar funcionalidades como descargas de Vimeo.
4. **Pruebas Avanzadas (Opcional)**: Si se necesita, escalar a VMs para entornos Intel reales.

## Riesgos y Consideraciones
- **Tamaño del binario**: Puede aumentar ligeramente debido a múltiples arquitecturas, pero es estándar.
- **Dependencias**: Qt6 debe soportar las versiones mínimas; si hay errores, ajustaremos.
- **Limpieza**: La carpeta temporal se puede borrar después de pruebas.
- **Requisitos**: Asegúrate de tener Xcode instalado para cross-compilación.

## Resultados de la Implementación
- **Compilación Exitosa**: Binario universal generado con arquitecturas x86_64 (Intel) y arm64 (Apple Silicon).
- **Deployment Target**: macOS 10.15+ (ajustado por compatibilidad con Qt 6.8.2).
- **Pruebas Básicas**: App compilada y ejecutada correctamente en entorno arm64. Verificación con `lipo` confirma soporte multi-arquitectura.
- **Logs**: Guardados en `temp_pruebas_compatibility/log_pruebas.txt`.

## Próximos Pasos
- Para pruebas en Intel reales, considera usar VMs o cross-compilación adicional.
- La carpeta `temp_pruebas_compatibility` se puede borrar si no la necesitas.
- Si surgen errores en entornos específicos, ajustar deployment target o versiones de Qt.

Este plan asegura que VimeoDownloader funcione en macOS 10.13+ en ambos tipos de hardware.
