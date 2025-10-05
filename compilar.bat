@echo off
echo Compilando VimeoDownloader...

REM Matar el proceso VimeoDownloader si está en ejecución
taskkill /F /IM VimeoDownloader.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Proceso VimeoDownloader terminado.
    timeout /t 1 >nul
) else (
    echo No se encontró el proceso VimeoDownloader en ejecución.
)

REM Añadir Qt y MinGW al PATH
set PATH=%PATH%;C:\Qt\6.8.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin

REM Crear directorio de compilación si no existe
if not exist build mkdir build
cd build

REM Configurar el proyecto
echo Configurando con CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/mingw_64"

REM Compilar el proyecto
echo Compilando VimeoDownloader...
cmake --build .

REM Verificar si la compilación fue exitosa
if %ERRORLEVEL% neq 0 (
    echo.
    echo Error en la compilación. Verifique los mensajes de error.
    cd ..
    exit /b 1
)

REM Crear carpeta tools si no existe y copiar herramientas
echo.
echo Preparando carpeta tools...
if not exist tools mkdir tools
if exist ..\tools\*.* (
    echo Copiando herramientas desde carpeta tools del proyecto...
    copy /Y ..\tools\*.* tools\
) else (
    echo Carpeta tools del proyecto no encontrada o vacía.
)

REM Ejecutar la aplicación
echo.
echo Compilación completada exitosamente.
echo Ejecutando VimeoDownloader...
echo.
start VimeoDownloader.exe

cd ..
