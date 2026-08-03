; ARCHIVO GENERADO por instalador.bat - no editar a mano, se pisa en cada corrida. 
[Setup] 
AppId=VideoDownloader 
AppName=VideoDownloader 
AppVersion=0.89 
DefaultDirName=C:\Portable\LGA\VideoDownloader 
DefaultGroupName=VideoDownloader 
UninstallDisplayIcon={app}\VideoDownloader.exe 
Compression=lzma2 
SolidCompression=yes 
OutputDir=installer 
OutputBaseFilename=VideoDownloader_Setup 
PrivilegesRequired=lowest 
UsePreviousAppDir=no 
DirExistsWarning=no 
SetupIconFile=resources\icons\LGA_VideoDownloader.ico 
 
[Files] 
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs 
 
[Icons] 
Name: "{group}\VideoDownloader"; Filename: "{app}\VideoDownloader.exe" 
Name: "{userdesktop}\VideoDownloader"; Filename: "{app}\VideoDownloader.exe"; Tasks: desktopicon 
 
[Tasks] 
Name: "desktopicon"; Description: "Crear un icono en el escritorio"; GroupDescription: "Iconos adicionales:" 
 
[Run] 
Filename: "{app}\VideoDownloader.exe"; Description: "Ejecutar VideoDownloader"; Flags: nowait postinstall skipifsilent 
 
[Code] 
const 
  LegacyUninstallKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\VimeoDownloader_is1'; 
  LegacyInstallDir = 'C:\Portable\LGA\VimeoDownloader'; 
 
function PrepareToInstall(var NeedsRestart: Boolean): String; 
var 
  ResultCode: Integer; 
begin 
  Result := ''; 
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM VideoDownloader.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); 
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM VimeoDownloader.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); 
  Sleep(1000); 
  RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, LegacyUninstallKey); 
  RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, LegacyUninstallKey); 
end; 
 
procedure CurStepChanged(CurStep: TSetupStep); 
begin 
  if CurStep = ssPostInstall then 
  begin 
    if DirExists(LegacyInstallDir) then 
    begin 
      if CompareText(LegacyInstallDir, ExpandConstant('{app}')) = 0 then 
        Exit; 
      DelTree(LegacyInstallDir, True, True, True); 
    end; 
  end; 
end; 
 
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep); 
var 
  ConfigPath: string; 
  ResultCode: Integer; 
begin 
  if CurUninstallStep = usPostUninstall then 
  begin 
    ConfigPath := ExpandConstant('{userappdata}\LGA\VideoDownloader'); 
    if DirExists(ConfigPath) then 
    begin 
      ResultCode := MsgBox('VideoDownloader ha guardado configuración en:' + #13#10 + ConfigPath + #13#10#13#10 + '¿Desea eliminar también esta configuración?', mbConfirmation, MB_YESNO); 
      if ResultCode = IDYES then 
      begin 
        if DelTree(ConfigPath, True, True, True) then 
          MsgBox('Configuración eliminada correctamente.', mbInformation, MB_OK) 
        else 
          MsgBox('No se pudo eliminar completamente la configuración.' + #13#10 + 'Puede eliminarla manualmente desde:' + #13#10 + ConfigPath, mbError, MB_OK); 
      end; 
    end; 
  end; 
end; 
