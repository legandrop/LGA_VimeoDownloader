@echo off
REM Eliminar carpeta deploy si existe para asegurar un entorno limpio
echo Limpiando carpeta de deploy anterior...
if exist deploy rmdir /S /Q deploy

echo Implementando VimeoDownloader...

REM Matar procesos previos si están en ejecución
taskkill /F /IM VimeoDownloader.exe 2>nul

REM Añadir Qt al PATH
set PATH=%PATH%;C:\Qt\6.8.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin

REM Crear directorio de implementación si no existe
if not exist deploy mkdir deploy

REM Compilar el proyecto en modo Release
cd build
echo Configurando con CMake (modo Release)...
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/mingw_64" -DCMAKE_BUILD_TYPE=Release
echo Compilando VimeoDownloader (modo Release)...
cmake --build . --config Release

REM Verificar si la compilación fue exitosa
if %ERRORLEVEL% neq 0 (
    echo.
    echo Error en la compilación. Verifique los mensajes de error.
    cd ..
    exit /b 1
)
cd ..

REM Copiar el ejecutable al directorio de implementación
copy /Y build\VimeoDownloader.exe deploy\

REM Usar windeployqt para copiar todas las DLLs de Qt necesarias
C:\Qt\6.8.2\mingw_64\bin\windeployqt.exe --release deploy\VimeoDownloader.exe

echo.
echo Implementacion completada exitosamente.
echo La aplicacion portable esta en la carpeta 'deploy'.
echo.

REM Ejecutar la aplicación implementada
echo Ejecutando VimeoDownloader...
start deploy\VimeoDownloader.exe
