# Mach1 plugin build and codesign

# local paths are in this file
include ./Makefile.variables

# getting OS type 
ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname)
endif

VERSION := $(shell grep VERSION: .github/workflows/workflow.yml | cut -d ':' -f 2 | cut -d ' ' -f 2 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')
BUNDLEID := $(shell grep BUNDLE_ID: .github/workflows/workflow.yml | cut -d ':' -f 2 | sed '1p;d' | cut -d ' ' -f 2 )

configure: 
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build build --config "Release"

codesign:
ifeq ($(detected_OS),Darwin)
	/usr/bin/codesign --timestamp --force --deep -s "Developer ID Application" build/M1-Notepad_artefacts/Release/AU/M1-Notepad.component -v
	/usr/bin/codesign --timestamp --force --deep -s "Developer ID Application" build/M1-Notepad_artefacts/Release/VST3/M1-Notepad.vst3 -v
	/usr/bin/codesign --timestamp --force --deep -s "Developer ID Application" build/M1-Notepad_artefacts/Release/AAX/M1-Notepad.aaxplugin -v
	$(WRAPTOOL) sign --verbose --account $(PACE_ID) --wcguid $(WRAP_GUID) --signid $(APPLE_CODESIGN_ID) --in build/M1-Notepad_artefacts/Release/AAX/M1-Notepad.aaxplugin --out build/M1-Notepad_artefacts/Release/AAX/M1-Notepad.aaxplugin
endif

package: get_bundle_id get_version build codesign
ifeq ($(detected_OS),Darwin)
	pkgbuild --identifier $(BUNDLE_ID).au --version $(VERSION) --component build/M1-Notepad_artefacts/Release/AU/M1-Notepad.component \
	--install-location "/Library/Audio/Plug-Ins/Components" build/M1-Notepad_artefacts/Release/M1-Notepad.au.pkg 
	pkgbuild --identifier $(BUNDLE_ID).vst3 --version $(VERSION) --component build/M1-Notepad_artefacts/Release/VST3/M1-Notepad.vst3 \
	--install-location "/Library/Audio/Plug-Ins/VST3" build/M1-Notepad_artefacts/Release/M1-Notepad.vst3.pkg 
	pkgbuild --identifier $(BUNDLE_ID).aaxplugin --version $(VERSION) --component build/M1-Notepad_artefacts/Release/AAX/M1-Notepad.aaxplugin \
	--install-location "/Library/Application\ Support/Avid/Audio/Plug-Ins" build/M1-Notepad_artefacts/Release/M1-Notepad.aaxplugin.pkg 
	productbuild --synthesize \
	--package "build/M1-Notepad_artefacts/Release/M1-Notepad.au.pkg" \
	--package "build/M1-Notepad_artefacts/Release/M1-Notepad.vst3.pkg" \
	--package "build/M1-Notepad_artefacts/Release/M1-Notepad.aaxplugin.pkg" \
	distribution.xml
	productbuild --sign "Developer ID Installer" --distribution distribution.xml --package-path build/M1-Notepad_artefacts/Release build/M1-Notepad_artefacts/Release/M1-Notepad.pkg
endif

get_bundle_id:
	echo " -- Using $(BUNDLEID)"

get_version:
	echo " -- Building version: $(VERSION)"