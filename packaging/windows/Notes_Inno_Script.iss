[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{44576435-E098-4581-A6D7-FD7FADC5B063}
AppName=Notes
AppVersion=0.9.0
AppPublisher=Awesomeness
AppPublisherURL=http://www.get-notes.com/
AppSupportURL=http://www.get-notes.com/
AppUpdatesURL=http://www.get-notes.com/
DefaultDirName={pf}\Notes 0.9.0
DefaultGroupName=Notes
OutputDir=C:\Users\user\Documents
OutputBaseFilename=NotesSetup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
;x-64
Source: "C:\Users\user\Documents\notes\bin\64\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\64\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
;x-86
Source: "C:\Users\user\Documents\notes\bin\32\Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode
Source: "C:\Users\user\Documents\notes\bin\32\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode

[Icons]
Name: "{group}\Notes"; Filename: "{app}\Notes.exe"
Name: "{group}\{cm:UninstallProgram,Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Notes"; Filename: "{app}\Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Notes.exe"; Description: "{cm:LaunchProgram,Notes}"; Flags: nowait postinstall skipifsilent

