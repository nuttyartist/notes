#define Version GetEnv("APP_VERSION")

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{44576435-E098-4581-A6D7-FD7FADC5B063}
AppName=Notes
AppVersion={#Version}
AppPublisher=Awesomeness
AppPublisherURL=https://www.get-notes.com/
AppSupportURL=https://www.get-notes.com/
AppUpdatesURL=https://www.get-notes.com/
UninstallDisplayIcon={app}\Notes.exe
DefaultDirName={code:GetDefaultInstallDir}\Notes
DefaultGroupName=Notes
UninstallDisplayName=Notes
OutputBaseFilename=NotesSetup_{#Version}
Compression=lzma
SolidCompression=yes
; Inno Setup can't seem to calculate the required disk space correctly, and asks for only 3.1 MB.
; Let's use an approximate value to represent the size of the Qt 6 build while installed on disk (100 MiB).
ExtraDiskSpaceRequired=104857600
OutputManifestFile=Setup-Manifest.txt

[Code]
function GetDefaultInstallDir(Param: string): string;
var
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  if IsWin64() and (Version.Major >= 10) and (Version.Minor >= 0) and (Version.Build >= 17763) then
    Result := ExpandConstant('{autopf64}')
  else
    Result := ExpandConstant('{autopf32}')
end;

function ParamExists(const Param: string): boolean;
var
  K: integer;
begin
  for K := 1 to ParamCount() do
  begin
    if SameText(ParamStr(K), Param) then
    begin
      Result := true;
      Exit;
    end;
  end;
  Result := false;
end;

// If a different version of Notes is already installed, this function will invoke its uninstaller.
// You can disable this behavior by invoking the installer via command line with /SKIPVERSIONCHECK.
function InitializeSetup(): boolean;
var
  ParamSilent, ParamVerySilent, ParamSuppressMsgBoxes: boolean;
  OurRegKey, Uninstaller, UninstallerParams, InstalledVersion: string;
  RootsToSearchIn: array of integer;
  K, ResultCode: integer;
begin
  if ParamExists('/SKIPVERSIONCHECK') then
  begin
    Result := true;
    Exit;
  end;
  ParamSilent := ParamExists('/SILENT');
  ParamVerySilent := ParamExists('/VERYSILENT');
  ParamSuppressMsgBoxes := ParamExists('/SUPPRESSMSGBOXES');
  OurRegKey := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}_is1');
  SetLength(RootsToSearchIn, 1);
  RootsToSearchIn[0] := HKLM32;
  // We have to also search in HKLM64, because some older versions of the installer ran in 64-bit install mode.
  if IsWin64() then
  begin
    SetLength(RootsToSearchIn, 2);
    RootsToSearchIn[1] := HKLM64;
  end;
  for K := 0 to (GetArrayLength(RootsToSearchIn) - 1) do
  begin
    if not RegQueryStringValue(RootsToSearchIn[K], OurRegKey, 'UninstallString', Uninstaller) then
    begin
      continue;
    end;
    if not RegQueryStringValue(RootsToSearchIn[K], OurRegKey, 'DisplayVersion', InstalledVersion) then
    begin
      continue;
    end;
    if SameText('{#SetupSetting("AppVersion")}', InstalledVersion) then
    begin
      continue;
    end;
    if not (ParamSilent or ParamVerySilent or ParamSuppressMsgBoxes) then
    begin
      if (MsgBox('Another version of {#SetupSetting("AppName")} is installed and must be removed first.'#13#13
                 'Don''t worry, all your notes will be safe.'#13#13
                 'Once the uninstall is complete, you''ll be prompted to install version {#SetupSetting("AppVersion")}.'#13#13
                 'Uninstall version ' + InstalledVersion + ' now?',
                 mbConfirmation, MB_YESNO) = IDNO) then
      begin
        Result := false;
        Exit;
      end;
    end;
    // We run the _uninstaller_ with /VERYSILENT by default.
    UninstallerParams := '/VERYSILENT';
    // If _this_ installer was invoked with /SILENT or /SUPPRESSMSGBOXES, we forward those to the uninstaller.
    if ParamSilent then
    begin
      UninstallerParams := UninstallerParams + ' /SILENT';
    end;
    if ParamSuppressMsgBoxes then
    begin
      UninstallerParams := UninstallerParams + ' /SUPPRESSMSGBOXES';
    end;
    if not Exec(RemoveQuotes(Uninstaller), UninstallerParams, '', SW_SHOW, ewWaitUntilTerminated, ResultCode)
       or (ResultCode <> 0) then
    begin
      // Error message boxes can't be suppressed.
      MsgBox('Failed to uninstall version ' + InstalledVersion + '. Reason:'#13#13
             + SysErrorMessage(ResultCode) + '. (code ' + IntToStr(ResultCode) + ')'#13#13
             'Try uninstalling it from the Windows Control Panel.',
             mbCriticalError, MB_OK);
      Result := false;
      Exit;
    end;
    if not (ParamSilent or ParamVerySilent or ParamSuppressMsgBoxes) then
    begin
      MsgBox('Uninstall of version ' + InstalledVersion + ' is complete.'#13#13
             'Installation of version {#SetupSetting("AppVersion")} will start after pressing OK...',
             mbInformation, MB_OK);
    end;
  end;
  Result := true;
end;

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
; Qt 6 build (64-bit)
; Minimum supported OS:
; - Windows 10 21H2 (1809 or later)
; Source: https://doc.qt.io/qt-6/supported-platforms.html#windows
Source: "{#SourcePath}\Notes64\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\QtQml\*"; DestDir: "{app}\QtQml"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\QtQuick\*"; DestDir: "{app}\QtQuick"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\libcrypto-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\libssl-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140_2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Qml.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QmlLocalStorage.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QmlModels.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QmlWorkerScript.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QmlXmlListModel.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Quick.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickControls2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickControls2Impl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickDialogs2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickDialogs2QuickImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickDialogs2Utils.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickLayouts.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickParticles.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickShapes.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6QuickTemplates2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763

; Qt 5 build (32-bit)
; Minimum supported OS:
; - Windows 7 (x86 and x86_64)
; Source: https://doc.qt.io/qt-5/supported-platforms.html#windows
Source: "{#SourcePath}\Notes32\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\QtGraphicalEffects\*"; DestDir: "{app}\QtGraphicalEffects"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\QtQml\*"; DestDir: "{app}\QtQml"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\QtQuick\*"; DestDir: "{app}\QtQuick"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\QtQuick.2\*"; DestDir: "{app}\QtQuick.2"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\scenegraph\*"; DestDir: "{app}\scenegraph"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\libcrypto-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\libGLESv2.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\libssl-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Qml.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5QmlModels.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5QmlWorkerScript.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Quick.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5QuickControls2.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5QuickParticles.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5QuickTemplates2.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5RemoteObjects.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763
Source: "{#SourcePath}\Notes32\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1; OnlyBelowVersion: 10.0.17763

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent
