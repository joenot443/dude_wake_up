; Nottawa.iss - Inno Setup 6 script for the Windows installer.
; Built by build_release.ps1, which passes:
;   /DAppVersion=<MAJOR.MINOR.PATCH>  /DStageDir=<staged folder>  /DOutputDir=<dist>
;
; Installs per user by default (no administrator prompt, into
; %LOCALAPPDATA%\Programs\Nottawa); "Install for all users" is offered too.
; The user's sessions, strands and settings live in %LOCALAPPDATA%\Nottawa
; and are left alone by upgrades and by uninstalling.

#ifndef AppVersion
  #error AppVersion is required (build with build_release.ps1)
#endif
#ifndef StageDir
  #error StageDir is required (build with build_release.ps1)
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

[Setup]
AppId={{A0F7C9B4-3F7E-4B0D-9F02-8B4A6D5C3E21}
AppName=Nottawa
AppVersion={#AppVersion}
AppVerName=Nottawa {#AppVersion}
AppPublisher=Joe Crozier
AppPublisherURL=https://nottawa.app
AppSupportURL=https://nottawa.app
VersionInfoVersion={#AppVersion}
DefaultDirName={autopf}\Nottawa
DefaultGroupName=Nottawa
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir={#OutputDir}
OutputBaseFilename=Nottawa-Setup-{#AppVersion}
SetupIconFile=..\..\app\nottawa_app\resources\nottawa.ico
UninstallDisplayIcon={app}\Nottawa.exe
UninstallDisplayName=Nottawa
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Windows 10 1903: the first with the UTF-8 code page the app's manifest asks for.
MinVersion=10.0.18362
; Close a running Nottawa (it saves its session) before replacing files.
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[InstallDelete]
; The previous version's bundled data, so files it no longer ships don't linger.
Type: filesandordirs; Name: "{app}\data"

[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\Nottawa"; Filename: "{app}\Nottawa.exe"
Name: "{autodesktop}\Nottawa"; Filename: "{app}\Nottawa.exe"; Tasks: desktopicon

[Registry]
; nottawa://strands/<slug> links open shared strands. HKA is the current
; user's hive for a per-user install, the machine's for an all-users one.
Root: HKA; Subkey: "Software\Classes\nottawa"; ValueType: string; ValueName: ""; ValueData: "URL:Nottawa"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\nottawa"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""
Root: HKA; Subkey: "Software\Classes\nottawa\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\Nottawa.exe"",0"
Root: HKA; Subkey: "Software\Classes\nottawa\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Nottawa.exe"" ""%1"""

[Run]
Filename: "{app}\Nottawa.exe"; Description: "{cm:LaunchProgram,Nottawa}"; Flags: nowait postinstall skipifsilent
