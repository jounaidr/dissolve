; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Dissolve"
#define MyAppVersion "0.4.8"
#define MyAppPublisher "Tristan Youngs"
#define MyAppURL "https://www.projectaten.com/"
#define MyAppExeName "Dissolve-GUI.exe"

; Locations of bin directories of Dissolve, Qt, GnuWin, MinGW etc.
#define DissolveDir GetEnv('DISSOLVE_DIR')
#define FreetypeDir GetEnv('FREETYPE_DIR')
#define FTGLDir GetEnv('FTGL_DIR')
#define QtDir GetEnv('Qt5_DIR')
#define MinGWDir GetEnv('MINGW_DIR')

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{72C93E43-B33F-4853-AFF7-44FF5C6FD94A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\Dissolve
DefaultGroupName={#MyAppName}
LicenseFile=..\..\LICENSE.txt
OutputDir=..\..\
OutputBaseFilename=Dissolve-0.4.8
SetupIconFile=Dissolve.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#DissolveDir}\Dissolve.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#DissolveDir}\Dissolve-GUI.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "Dissolve.ico"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: "{#FreetypeDir}\libfreetype.dll"; DestDir: "{app}\bin"
Source: "{#FTGLDir}\src\libftgl.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libgcc_s_seh-1.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libstdc++-6.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5OpenGL.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5PrintSupport.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\libEGL.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\libGLESv2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\bin\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\bin\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\*.dll"; DestDir: "{app}\bin\imageformats"; Flags: ignoreversion
; Windows 7
;Source: "C:\Windows\System32\D3DCompiler_43.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Windows 10
Source: "C:\Windows\System32\D3DCompiler_47.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; IconFilename: "{app}\bin\Dissolve.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#MyAppName}"; IconFilename: "{app}\bin\Dissolve.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
