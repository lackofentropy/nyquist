; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3E29123D-0726-441C-8A38-42836B05F74C}
AppName=NyquistIDE
AppVerName=NyquistIDE 3.06
AppPublisher=Roger B. Dannenberg
AppPublisherURL=http://www.cs.cmu.edu/~music/nyquist
AppSupportURL=http://www.cs.cmu.edu/~music/nyquist
AppUpdatesURL=http://www.cs.cmu.edu/~music/nyquist
DefaultDirName={pf}\Nyquist
DefaultGroupName=Nyquist
LicenseFile=C:\Users\rbd\nyquist\nyqrelide\license.txt
InfoAfterFile=C:\Users\rbd\nyquist\nyqrelide\Readme.txt
OutputDir=setup
OutputBaseFilename=setupnyqiderun
SetupIconFile=C:\Users\rbd\nyquist\sys\win\wingui\nycon.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\rbd\nyquist\nyqrelide\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKLM32; Subkey: "Software\CMU\Nyquist"; ValueType: string; ValueName: "XLISPPATH"; ValueData: "{app}\runtime,{app}\lib"

[Icons]
Name: "{group}\NyquistIDE"; Filename: "{app}\jnyqide.bat"; WorkingDir: "{app}"
Name: "{commondesktop}\NyquistIDE"; Filename: "{app}\jnyqide.bat"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\jnyqide.bat"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,NyquistIDE}"; Flags: shellexec postinstall skipifsilent

