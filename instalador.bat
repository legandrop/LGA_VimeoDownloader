@echo off
echo Preparando instalador para VimeoDownloader...

REM Verificar si ya existe la carpeta deploy
if not exist deploy (
    echo Error: La carpeta deploy no existe. Ejecute primero deploy.bat
    exit /b 1
)

REM Verificar si Inno Setup está instalado
set "INNO_PATH=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if not exist "%INNO_PATH%" (
    echo Inno Setup no encontrado. Descargando...
    
    REM Crear directorio temporal
    mkdir temp_inno
    cd temp_inno
    
    REM Descargar Inno Setup
    powershell -Command "& {Invoke-WebRequest -Uri 'https://jrsoftware.org/download.php/is.exe' -OutFile 'innosetup.exe'}"
    
    REM Instalar Inno Setup silenciosamente
    echo Instalando Inno Setup...
    start /wait innosetup.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
    
    cd ..
    rmdir /S /Q temp_inno
)

REM Crear el script de Inno Setup
echo Generando script de instalador...
echo [Setup] > VimeoDownloader_installer.iss
echo AppName=VimeoDownloader >> VimeoDownloader_installer.iss
echo AppVersion=0.83 >> VimeoDownloader_installer.iss
echo DefaultDirName=C:\Portable\LGA\VimeoDownloader >> VimeoDownloader_installer.iss
echo DefaultGroupName=VimeoDownloader >> VimeoDownloader_installer.iss
echo UninstallDisplayIcon={app}\VimeoDownloader.exe >> VimeoDownloader_installer.iss
echo Compression=lzma2 >> VimeoDownloader_installer.iss
echo SolidCompression=yes >> VimeoDownloader_installer.iss
echo OutputDir=installer >> VimeoDownloader_installer.iss
echo OutputBaseFilename=VimeoDownloader_Setup >> VimeoDownloader_installer.iss
echo PrivilegesRequired=lowest >> VimeoDownloader_installer.iss
echo UsePreviousAppDir=no >> VimeoDownloader_installer.iss
echo DirExistsWarning=no >> VimeoDownloader_installer.iss

REM Añadir recursos solo si existen
if exist resources\icons\LGA_VimeoDownloader.ico (
    echo SetupIconFile=resources\icons\LGA_VimeoDownloader.ico >> VimeoDownloader_installer.iss
)

echo. >> VimeoDownloader_installer.iss
echo [Files] >> VimeoDownloader_installer.iss
echo Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs >> VimeoDownloader_installer.iss
echo. >> VimeoDownloader_installer.iss
echo [Icons] >> VimeoDownloader_installer.iss
echo Name: "{group}\VimeoDownloader"; Filename: "{app}\VimeoDownloader.exe" >> VimeoDownloader_installer.iss
echo Name: "{userdesktop}\VimeoDownloader"; Filename: "{app}\VimeoDownloader.exe"; Tasks: desktopicon >> VimeoDownloader_installer.iss
echo. >> VimeoDownloader_installer.iss
echo [Tasks] >> VimeoDownloader_installer.iss
echo Name: "desktopicon"; Description: "Crear un icono en el escritorio"; GroupDescription: "Iconos adicionales:" >> VimeoDownloader_installer.iss
echo. >> VimeoDownloader_installer.iss
echo [Run] >> VimeoDownloader_installer.iss
echo Filename: "{app}\VimeoDownloader.exe"; Description: "Ejecutar VimeoDownloader"; Flags: nowait postinstall skipifsilent >> VimeoDownloader_installer.iss

REM Añadir código Pascal para preguntar sobre eliminar configuración durante desinstalación
echo. >> VimeoDownloader_installer.iss
echo [Code] >> VimeoDownloader_installer.iss
echo procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep); >> VimeoDownloader_installer.iss
echo var >> VimeoDownloader_installer.iss
echo   ConfigPath: string; >> VimeoDownloader_installer.iss
echo   ResultCode: Integer; >> VimeoDownloader_installer.iss
echo begin >> VimeoDownloader_installer.iss
echo   if CurUninstallStep = usPostUninstall then >> VimeoDownloader_installer.iss
echo   begin >> VimeoDownloader_installer.iss
echo     ConfigPath := ExpandConstant('{userappdata}\LGA\VimeoDownloader'); >> VimeoDownloader_installer.iss
echo     if DirExists(ConfigPath) then >> VimeoDownloader_installer.iss
echo     begin >> VimeoDownloader_installer.iss
echo       ResultCode := MsgBox('VimeoDownloader ha guardado configuración en:' + #13#10 + ConfigPath + #13#10#13#10 + '¿Desea eliminar también esta configuración?', mbConfirmation, MB_YESNO); >> VimeoDownloader_installer.iss
echo       if ResultCode = IDYES then >> VimeoDownloader_installer.iss
echo       begin >> VimeoDownloader_installer.iss
echo         if DelTree(ConfigPath, True, True, True) then >> VimeoDownloader_installer.iss
echo           MsgBox('Configuración eliminada correctamente.', mbInformation, MB_OK) >> VimeoDownloader_installer.iss
echo         else >> VimeoDownloader_installer.iss
echo           MsgBox('No se pudo eliminar completamente la configuración.' + #13#10 + 'Puede eliminarla manualmente desde:' + #13#10 + ConfigPath, mbError, MB_OK); >> VimeoDownloader_installer.iss
echo       end; >> VimeoDownloader_installer.iss
echo     end; >> VimeoDownloader_installer.iss
echo   end; >> VimeoDownloader_installer.iss
echo end; >> VimeoDownloader_installer.iss

REM Crear directorio para el instalador si no existe
if not exist installer mkdir installer

REM Compilar el instalador
echo Compilando el instalador...
"%INNO_PATH%" VimeoDownloader_installer.iss

if %ERRORLEVEL% neq 0 (
    echo Error al compilar el instalador.
    exit /b 1
)

echo.
echo Instalador creado exitosamente en la carpeta 'installer'.
echo Archivo: installer\VimeoDownloader_Setup.exe 
echo. 

choice /C YN /M "¿Desea ejecutar el instalador ahora mismo?"
if %ERRORLEVEL%==1 (
    echo Ejecutando el instalador...
    start "" "installer\VimeoDownloader_Setup.exe"
) else (
    echo Instalador no ejecutado.
)
