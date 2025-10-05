@echo off
echo Limpiando VimeoDownloader...

REM Eliminar directorio de compilación
if exist build (
    echo Eliminando directorio de compilación...
    rmdir /s /q build
)

echo.
echo Limpieza completada.
echo.
