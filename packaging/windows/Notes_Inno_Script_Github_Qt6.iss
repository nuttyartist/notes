#define Version GetEnv("BUILD_VERSION")

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
DefaultDirName={autopf}\Notes
DefaultGroupName=Notes
UninstallDisplayName=Notes
OutputBaseFilename=NotesSetup_{#Version}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
;x-64
Source: "{#SourcePath}\Notes64\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\libcrypto-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\libssl-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\networkinformation\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\styles\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\tls\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "{#SourcePath}\Notes64\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent
