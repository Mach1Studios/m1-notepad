cmake_minimum_required(VERSION 3.13.0)
project(M1-Notepad VERSION 1.0.2)
add_subdirectory(JUCE)

if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/JUCE_deps)
    set(VST2_PATH "JUCE_deps/VST_SDK_vst2/VST2_SDK")
    message("-- JUCE Dependencies Found: ${PROJECT_SOURCE_DIR}/JUCE_deps")
else()
    message("Warning: Did not find additional JUCE deps submodule. Please add the appropriate SDKs manually.")
endif()

# check which formats we want to build
set(FORMATS "VST3")
set(AAX_PATH "JUCE/modules/juce_audio_plugin_client/AAX/SDK")
juce_set_aax_sdk_path(${AAX_PATH})
list(APPEND FORMATS "AAX")

# if (VST2_PATH)
#     list(APPEND FORMATS "VST2")
#     juce_set_vst2_sdk_path(${VST2_PATH})
# endif()

if (APPLE)
    list(APPEND FORMATS "AU")
endif()

# add the plugin targets
juce_add_plugin(M1-Notepad
                VERSION "1.0.2"
                COMPANY_NAME "Mach1"
                PLUGIN_MANUFACTURER_CODE "Mac1"
                PLUGIN_CODE "M1NP"
                FORMATS ${FORMATS}
                VST3_CATEGORIES "Tools"
                AAX_CATEGORY "AAX_ePlugInCategory_None"
                AU_MAIN_TYPE "kAudioUnitType_Effect"
                COMPANY_WEBSITE "https://mach1.tech"
                COMPANY_EMAIL "whatsup@mach1.tech"
                BUNDLE_ID "com.mach1.notepad"
                PLUGIN_NAME "M1-Notepad"
                PRODUCT_NAME "M1-Notepad")

# add the sources
add_subdirectory(Resources)
add_subdirectory(Source)

# add required flags
target_link_libraries(M1-Notepad PRIVATE juce::juce_recommended_warning_flags juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
target_link_libraries(M1-Notepad PRIVATE juce::juce_audio_utils)
target_compile_definitions(M1-Notepad 
    PUBLIC 
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0)
juce_generate_juce_header(M1-Notepad)

# setup the copying to the output folder
if (APPLE)
    set(COPY_FOLDER ${CMAKE_SOURCE_DIR}/Builds/MacOSX)
elseif(WIN32)
    set(COPY_FOLDER ${CMAKE_SOURCE_DIR}/Builds/VisualStudio2019)
endif()

foreach(FORMAT ${FORMATS})
    get_target_property(ARTEFACTS_DIR M1-Notepad_${FORMAT} LIBRARY_OUTPUT_DIRECTORY)
    add_custom_command(TARGET M1-Notepad_${FORMAT} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARTEFACTS_DIR} ${COPY_FOLDER})
endforeach()