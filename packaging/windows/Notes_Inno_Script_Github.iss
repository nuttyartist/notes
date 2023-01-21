#define Version GetEnv("APP_VERSION")

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

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Qt 6 build (64-bit)
; Minimum supported OS:
; - Windows 10 21H2 (1809 or later)
; Source: https://doc.qt.io/qt-6/supported-platforms.html#windows
Source: "{#SourcePath}\Notes64\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\libcrypto-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\libssl-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcp140_2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\networkinformation\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\styles\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\tls\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\Notes64\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
; Qt 5 build (32-bit)
; Minimum supported OS:
; - Windows 7 (x86 and x86_64)
; Source: https://doc.qt.io/qt-5/supported-platforms.html#windows
Source: "{#SourcePath}\Notes32\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\libcrypto-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\libssl-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\styles\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1
Source: "{#SourcePath}\Notes32\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; MinVersion: 6.1

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent
