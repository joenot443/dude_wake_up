; Nottawa.iss — Inno Setup 6 script
; Invoked by build_release.ps1 with these /D defines:
;   AppVersion=...  AppChannel=preview|release
;   StageDir=<absolute path to staged tree>
;   OutputDir=<absolute path for output installer>
;   OutputBaseFilename=<name without .exe>

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef AppChannel
  #define AppChannel "preview"
#endif
#ifndef StageDir
  #define StageDir "..\..\dist\stage-preview"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif
#ifndef OutputBaseFilename
  #define OutputBaseFilename "Nottawa-Setup-" + AppVersion + "-" + AppChannel
#endif

[Setup]
AppId={{A0F7C9B4-3F7E-4B0D-9F02-8B4A6D5C3E21}
AppName=Nottawa
AppVersion={#AppVersion}
AppPublisher=Joe Crozier
AppPublisherURL=https://nottawa.app
DefaultDirName={autopf}\Nottawa
DefaultGroupName=Nottawa
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename={#OutputBaseFilename}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
MinVersion=10.0.19041
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=commandline dialog
UninstallDisplayIcon={app}\Nottawa.exe
; Long-path opt-in: helps installer place under long user paths.
; Application-side long-path awareness comes from app.manifest.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#StageDir}\Nottawa.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StageDir}\*.dll";       DestDir: "{app}"; Flags: ignoreversion
Source: "{#StageDir}\data\*";      DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Nottawa"; Filename: "{app}\Nottawa.exe"
Name: "{group}\Uninstall Nottawa"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Nottawa"; Filename: "{app}\Nottawa.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Registry]
; nottawa:// URL scheme handler. Full implementation and argv routing land
; in Phase 5 (URLSchemeHandler_win.cpp). Registered per-user so no admin
; escalation is required.
Root: HKCU; Subkey: "Software\Classes\nottawa"; ValueType: string; ValueName: ""; ValueData: "URL:Nottawa Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\nottawa"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""
Root: HKCU; Subkey: "Software\Classes\nottawa\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Nottawa.exe"" ""%1"""

[Run]
Filename: "{app}\Nottawa.exe"; Description: "Launch Nottawa"; Flags: nowait postinstall skipifsilent
