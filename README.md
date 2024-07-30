# Mach1 M1-Notepad Plugin
NotePad based plugin for track/session notes, very basic and just passes through audio on a processing level

### Installation
- unzip and place the plugin in the appropriate folders
  - AU `/Library/Audio/Plug-Ins/Components`
  - VST `/Library/Audio/Plug-Ins/VST`
  - VST3 `/Library/Audio/Plug-Ins/VST3`
  - AAX `/Library/Application Support/Avid/Audio/Plug-Ins`
  
### Notes
- Does not include an UndoManager or system, to avoid flooding your project/DAW with undo steps for text changes, lets keep undo for audio changes only!
- Reaper will block some keystrokes, if you are using Reaper please `Open the FX window and from FX menu enable “send all keyboard input to plugin”`

### TODO
- ~resize-able window (on supported DAWS)~
- ~AAX resize function/button?~
- Strikethrough
- List Maker
- Use new webview from JUCE8
- Fix AAX CI building (lack of PACE AAX SIGNING tool for CI but this can be done by reversing [tag:](https://github.com/Mach1Studios/m1-notepad/releases/tag/feature%2Fci-for-aax))
