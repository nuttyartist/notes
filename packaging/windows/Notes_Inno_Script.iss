#define PLATFORMENV GetEnv("PLATFORMENV")
#define Version GetEnv("APPVEYOR_BUILD_VERSION")

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{44576435-E098-4581-A6D7-FD7FADC5B063}
AppName=Notes
AppVersion={#Version}
AppPublisher=Awesomeness
AppPublisherURL=http://www.get-notes.com/
AppSupportURL=http://www.get-notes.com/
AppUpdatesURL=http://www.get-notes.com/
DefaultDirName={pf}\Notes
DefaultGroupName=Notes
OutputDir=C:\ci\notes\installer
OutputBaseFilename=NotesSetup_{#Version}_{#PLATFORMENV}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\ci\notes\bin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent

