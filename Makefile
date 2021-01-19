# Mach1 plugin build and codesign

# local paths are in this file
include ~/m1-globallocal.mk

# getting OS type 
ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname)
endif

insstall: 

build: 
	$(JUCE_CLI) --status "./m1-notepad.jucer"
	$(JUCE_CLI) --resave "./m1-notepad.jucer"
	xcodebuild -project ./Builds/MacOSX/M1-NotePad.xcodeproj -scheme "M1-NotePad - All" -configuration Release clean
	xcodebuild -project ./Builds/MacOSX/M1-NotePad.xcodeproj -scheme "M1-NotePad - All" -configuration Release build
	# aax
	$(WRAPTOOL) sign --verbose --account $(PACE_ID) --wcguid $(M1_GLOBAL_GUID) --signid $(APPLE_ID) --in /Library/Application\ Support/Avid/Audio/Plug-Ins/M1-NotePad.aaxplugin --out /Library/Application\ Support/Avid/Audio/Plug-Ins/M1-NotePad.aaxplugin --autoinstall on

codesign:
	$(WRAPTOOL) sign --verbose --account $(PACE_ID) --wcguid $(M1_GLOBAL_GUID) --signid $(APPLE_ID) --in /Library/Application\ Support/Avid/Audio/Plug-Ins/M1-NotePad.aaxplugin --out /Library/Application\ Support/Avid/Audio/Plug-Ins/M1-NotePad.aaxplugin --autoinstall on
