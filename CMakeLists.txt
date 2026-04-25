cmake_minimum_required(VERSION 3.22)
project(ymvst VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ymvst
    COMPANY_NAME "ymvst-mac"
    PLUGIN_MANUFACTURER_CODE Ymvm
    PLUGIN_CODE YmV1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ymVST"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

target_sources(ymvst
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/YmEngine.cpp
        src/gui/BitmapFont.cpp
        src/gui/RetroLookAndFeel.cpp
        src/gui/BeveledButton.cpp
        src/gui/SpinnerControl.cpp
        src/gui/SectionLabel.cpp
        src/gui/WaveformEditor.cpp
        src/gui/ScopeDisplay.cpp
        src/gui/HardwareWaveformSelector.cpp
        libs/ayumi/ayumi.c
)

target_include_directories(ymvst
    PRIVATE
        libs/ayumi
        src
)

target_compile_definitions(ymvst
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(ymvst
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
