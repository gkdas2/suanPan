﻿; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "suanPan"
#define MyAppVersion "2.5"
#define MyAppPublisher "Theodore Chang"
#define MyAppURL "https://github.com/TLCFEM/suanPan"
#define MyAppExeName "suanPan.exe"
#define MyAppAssocName MyAppName + " Model"
#define MyAppAssocExt ".sp"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt
#define StoragePath "..\..\suanPan-win-mkl-vtk\"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{5DA1E99B-205D-435A-80C8-BE32407DE4D2}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf64}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=.\LICENSE.txt
InfoAfterFile=.\POST.txt
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir={#StoragePath}\installer
OutputBaseFilename=suanPan-win-mkl-vtk
SetupIconFile=..\Resource\suanPan-Papirus.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "updater"; Description: "Include updater.exe"
Name: "vcredist"; Description: "Include Microsoft Visual C++ Redistributable *.dll"

[Files]
Source: "..\..\suanPan-vs\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}libifcoremd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}libiomp5md.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}libmmd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}svml_dispmd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}tbb12.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}tbbmalloc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}tbbmalloc_proxy.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StoragePath}updater.exe"; DestDir: "{app}"; Flags: ignoreversion; Tasks: updater
Source: "{#StoragePath}msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; Tasks: vcredist
Source: "{#StoragePath}msvcp140_atomic_wait.dll"; DestDir: "{app}"; Flags: ignoreversion; Tasks: vcredist
Source: "{#StoragePath}vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; Tasks: vcredist
Source: "{#StoragePath}vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Tasks: vcredist
Source: ".\AddAssociation.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\suanPan.sublime-completions"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\suanPan.sublime-syntax"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""-f"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".sp"; ValueData: ""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

