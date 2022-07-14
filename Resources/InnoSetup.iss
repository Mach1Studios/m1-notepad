[Setup]
AppName=M1-Notepad
AppVersion=1.0.1
Uninstallable=no
SourceDir="{#BuildDir}"
DefaultDirName="{commoncf64}\VST3"
DefaultGroupName=M1-Notepad
OutputBaseFilename=M1-Notepad_WIN
[Files]
Source: "VST3\M1-Notepad.vst3"; DestDir: {app}; Flags: recursesubdirs
Source: "VST\M1-Notepad.dll"; DestDir: {cf64}\VST
Source: "AAX\M1-Notepad.aaxplugin\*"; DestDir: "{commoncf64}\Avid\Audio\Plug-Ins\M1-Notepad.aaxplugin"; Flags: recursesubdirs