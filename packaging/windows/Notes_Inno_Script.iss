[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{44576435-E098-4581-A6D7-FD7FADC5B063}
AppName=Notes
AppVersion=0.8.0
AppPublisher=Awesomeness
AppPublisherURL=http://www.get-notes.com/
AppSupportURL=http://www.get-notes.com/
AppUpdatesURL=http://www.get-notes.com/
DefaultDirName={pf}\Notes 0.8.0
DefaultGroupName=Notes
OutputDir=C:\Users\user\Documents
OutputBaseFilename=NotesSetup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\user\Documents\notes\bin\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\user\Documents\notes\bin\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\user\Documents\notes\bin\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\user\Documents\notes\bin\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\user\Documents\notes\bin\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent

