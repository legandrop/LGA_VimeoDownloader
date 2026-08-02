@echo off
echo Limpiando VideoDownloader...

REM Eliminar directorio de compilación
if exist build (
    echo Eliminando directorio de compilación...
    rmdir /s /q build
)

echo.
echo Limpieza completada.
echo.
