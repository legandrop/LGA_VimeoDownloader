[Setup] 
AppName=VimeoDownloader 
AppVersion=0.83 
DefaultDirName=C:\Portable\LGA\VimeoDownloader 
DefaultGroupName=VimeoDownloader 
UninstallDisplayIcon={app}\VimeoDownloader.exe 
Compression=lzma2 
SolidCompression=yes 
OutputDir=installer 
OutputBaseFilename=VimeoDownloader_Setup 
PrivilegesRequired=lowest 
UsePreviousAppDir=no 
DirExistsWarning=no 
SetupIconFile=resources\icons\LGA_VimeoDownloader.ico 
 
[Files] 
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs 
 
[Icons] 
Name: "{group}\VimeoDownloader"; Filename: "{app}\VimeoDownloader.exe" 
Name: "{userdesktop}\VimeoDownloader"; Filename: "{app}\VimeoDownloader.exe"; Tasks: desktopicon 
 
[Tasks] 
Name: "desktopicon"; Description: "Crear un icono en el escritorio"; GroupDescription: "Iconos adicionales:" 
 
[Run] 
Filename: "{app}\VimeoDownloader.exe"; Description: "Ejecutar VimeoDownloader"; Flags: nowait postinstall skipifsilent 
 
[Code] 
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep); 
var 
  ConfigPath: string; 
  ResultCode: Integer; 
begin 
  if CurUninstallStep = usPostUninstall then 
  begin 
    ConfigPath := ExpandConstant('{userappdata}\LGA\VimeoDownloader'); 
    if DirExists(ConfigPath) then 
    begin 
      ResultCode := MsgBox('VimeoDownloader ha guardado configuración en:' + #13#10 + ConfigPath + #13#10#13#10 + '¿Desea eliminar también esta configuración?', mbConfirmation, MB_YESNO); 
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
