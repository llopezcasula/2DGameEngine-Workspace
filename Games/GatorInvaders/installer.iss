[Setup]
AppName=Gator Invaders
AppVersion=1.0
DefaultDirName={autopf}\Gator Invaders
DefaultGroupName=Gator Invaders
OutputDir=dist
OutputBaseFilename=GatorInvadersSetup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=icon.ico


[Files]
Source: "GatorInvaders.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "icon.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Icons]
Name: "{group}\Gator Invaders"; Filename: "{app}\GatorInvaders.exe"; WorkingDir: "{app}"; IconFilename: "{app}\icon.ico"
Name: "{commondesktop}\Gator Invaders"; Filename: "{app}\GatorInvaders.exe"; WorkingDir: "{app}"; IconFilename: "{app}\icon.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\GatorInvaders.exe"; Description: "Launch Gator Invaders"; Flags: nowait postinstall skipifsilent
