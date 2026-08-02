@echo off
echo Preparando instalador para VideoDownloader...

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
echo ; ARCHIVO GENERADO por instalador.bat - no editar a mano, se pisa en cada corrida. > VideoDownloader_installer.iss
echo [Setup] >> VideoDownloader_installer.iss
echo AppId=VideoDownloader >> VideoDownloader_installer.iss
echo AppName=VideoDownloader >> VideoDownloader_installer.iss
REM Mantener sincronizado con el project() de CMakeLists.txt.
echo AppVersion=0.89 >> VideoDownloader_installer.iss
echo DefaultDirName=C:\Portable\LGA\VideoDownloader >> VideoDownloader_installer.iss
echo DefaultGroupName=VideoDownloader >> VideoDownloader_installer.iss
echo UninstallDisplayIcon={app}\VideoDownloader.exe >> VideoDownloader_installer.iss
echo Compression=lzma2 >> VideoDownloader_installer.iss
echo SolidCompression=yes >> VideoDownloader_installer.iss
echo OutputDir=installer >> VideoDownloader_installer.iss
echo OutputBaseFilename=VideoDownloader_Setup >> VideoDownloader_installer.iss
echo PrivilegesRequired=lowest >> VideoDownloader_installer.iss
echo UsePreviousAppDir=no >> VideoDownloader_installer.iss
echo DirExistsWarning=no >> VideoDownloader_installer.iss

REM Añadir recursos solo si existen
if exist resources\icons\LGA_VideoDownloader.ico (
    echo SetupIconFile=resources\icons\LGA_VideoDownloader.ico >> VideoDownloader_installer.iss
)

echo. >> VideoDownloader_installer.iss
echo [Files] >> VideoDownloader_installer.iss
echo Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo [Icons] >> VideoDownloader_installer.iss
echo Name: "{group}\VideoDownloader"; Filename: "{app}\VideoDownloader.exe" >> VideoDownloader_installer.iss
echo Name: "{userdesktop}\VideoDownloader"; Filename: "{app}\VideoDownloader.exe"; Tasks: desktopicon >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo [Tasks] >> VideoDownloader_installer.iss
echo Name: "desktopicon"; Description: "Crear un icono en el escritorio"; GroupDescription: "Iconos adicionales:" >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo [Run] >> VideoDownloader_installer.iss
echo Filename: "{app}\VideoDownloader.exe"; Description: "Ejecutar VideoDownloader"; Flags: nowait postinstall skipifsilent >> VideoDownloader_installer.iss

REM Añadir código Pascal para preguntar sobre eliminar configuración durante desinstalación
echo. >> VideoDownloader_installer.iss
echo [Code] >> VideoDownloader_installer.iss
REM La app se llamaba VimeoDownloader y el instalador no definia AppId, asi que
REM Inno usaba el AppName como identificador. Al renombrar, la entrada vieja de
REM Programas y caracteristicas queda huerfana apuntando a la carpeta anterior:
REM si el usuario la desinstala desde ahi, borra una instalacion que ya no existe
REM y deja la nueva sin registrar. Se borra la clave y la carpeta vieja.
echo const >> VideoDownloader_installer.iss
echo   LegacyUninstallKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\VimeoDownloader_is1'; >> VideoDownloader_installer.iss
echo   LegacyInstallDir = 'C:\Portable\LGA\VimeoDownloader'; >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo function PrepareToInstall(var NeedsRestart: Boolean): String; >> VideoDownloader_installer.iss
echo var >> VideoDownloader_installer.iss
echo   ResultCode: Integer; >> VideoDownloader_installer.iss
echo begin >> VideoDownloader_installer.iss
echo   Result := ''; >> VideoDownloader_installer.iss
echo   Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM VideoDownloader.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); >> VideoDownloader_installer.iss
echo   Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM VimeoDownloader.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); >> VideoDownloader_installer.iss
echo   Sleep(1000); >> VideoDownloader_installer.iss
echo   RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, LegacyUninstallKey); >> VideoDownloader_installer.iss
echo   RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, LegacyUninstallKey); >> VideoDownloader_installer.iss
echo end; >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo procedure CurStepChanged(CurStep: TSetupStep); >> VideoDownloader_installer.iss
echo begin >> VideoDownloader_installer.iss
echo   if CurStep = ssPostInstall then >> VideoDownloader_installer.iss
echo   begin >> VideoDownloader_installer.iss
echo     if DirExists(LegacyInstallDir) then >> VideoDownloader_installer.iss
echo     begin >> VideoDownloader_installer.iss
echo       if CompareText(LegacyInstallDir, ExpandConstant('{app}')) = 0 then >> VideoDownloader_installer.iss
echo         Exit; >> VideoDownloader_installer.iss
echo       DelTree(LegacyInstallDir, True, True, True); >> VideoDownloader_installer.iss
echo     end; >> VideoDownloader_installer.iss
echo   end; >> VideoDownloader_installer.iss
echo end; >> VideoDownloader_installer.iss
echo. >> VideoDownloader_installer.iss
echo procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep); >> VideoDownloader_installer.iss
echo var >> VideoDownloader_installer.iss
echo   ConfigPath: string; >> VideoDownloader_installer.iss
echo   ResultCode: Integer; >> VideoDownloader_installer.iss
echo begin >> VideoDownloader_installer.iss
echo   if CurUninstallStep = usPostUninstall then >> VideoDownloader_installer.iss
echo   begin >> VideoDownloader_installer.iss
echo     ConfigPath := ExpandConstant('{userappdata}\LGA\VideoDownloader'); >> VideoDownloader_installer.iss
echo     if DirExists(ConfigPath) then >> VideoDownloader_installer.iss
echo     begin >> VideoDownloader_installer.iss
echo       ResultCode := MsgBox('VideoDownloader ha guardado configuración en:' + #13#10 + ConfigPath + #13#10#13#10 + '¿Desea eliminar también esta configuración?', mbConfirmation, MB_YESNO); >> VideoDownloader_installer.iss
echo       if ResultCode = IDYES then >> VideoDownloader_installer.iss
echo       begin >> VideoDownloader_installer.iss
echo         if DelTree(ConfigPath, True, True, True) then >> VideoDownloader_installer.iss
echo           MsgBox('Configuración eliminada correctamente.', mbInformation, MB_OK) >> VideoDownloader_installer.iss
echo         else >> VideoDownloader_installer.iss
echo           MsgBox('No se pudo eliminar completamente la configuración.' + #13#10 + 'Puede eliminarla manualmente desde:' + #13#10 + ConfigPath, mbError, MB_OK); >> VideoDownloader_installer.iss
echo       end; >> VideoDownloader_installer.iss
echo     end; >> VideoDownloader_installer.iss
echo   end; >> VideoDownloader_installer.iss
echo end; >> VideoDownloader_installer.iss

REM Crear directorio para el instalador si no existe
if not exist installer mkdir installer

REM Compilar el instalador
echo Compilando el instalador...
"%INNO_PATH%" VideoDownloader_installer.iss

if %ERRORLEVEL% neq 0 (
    echo Error al compilar el instalador.
    exit /b 1
)

echo.
echo Instalador creado exitosamente en la carpeta 'installer'.
echo Archivo: installer\VideoDownloader_Setup.exe 
echo. 

choice /C YN /M "¿Desea ejecutar el instalador ahora mismo?"
if %ERRORLEVEL%==1 (
    echo Ejecutando el instalador...
    start "" "installer\VideoDownloader_Setup.exe"
) else (
    echo Instalador no ejecutado.
)
