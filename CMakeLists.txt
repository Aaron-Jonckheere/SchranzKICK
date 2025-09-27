cmake_minimum_required(VERSION 3.15)
project(SchranzKick VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# You must have JUCE installed and its CMake package available
# e.g. on Windows set JUCE_DIR to the path containing JUCEConfig.cmake
#   -DJUCE_DIR="C:/path/to/JUCE/cmake"

find_package(JUCE CONFIG REQUIRED COMPONENTS juce_audio_utils juce_audio_processors juce_dsp juce_gui_extra)

juce_add_plugin(SchranzKick
    COMPANY_NAME "OpenAudio"
    PLUGIN_NAME  "SchranzKick"
    BUNDLE_ID    com.openaudio.schranzkick
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE OAio
    PLUGIN_CODE Skik
    FORMATS VST3
    PRODUCT_NAME "SchranzKick")

juce_generate_juce_header(SchranzKick)

set(SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/KickVoice.cpp
    Source/DSP/OnePole.cpp
    Source/DSP/Distortion.cpp
)

set(HEADERS
    Source/PluginProcessor.h
    Source/PluginEditor.h
    Source/DSP/KickVoice.h
    Source/DSP/OnePole.h
    Source/DSP/Distortion.h
)


add_library(SchranzKickLib STATIC ${SOURCES} ${HEADERS})

# Add JUCE modules to the static library as well

target_link_libraries(SchranzKickLib PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
)

target_compile_definitions(SchranzKickLib PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# The actual plugin target links to our static lib

target_sources(SchranzKick PRIVATE ${SOURCES} ${HEADERS})

target_link_libraries(SchranzKick PRIVATE
    juce::juce_gui_extra
    juce::juce_dsp
)

# Simpler: just link juce modules directly to the plugin

target_link_libraries(SchranzKick PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
)

target_compile_definitions(SchranzKick PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
