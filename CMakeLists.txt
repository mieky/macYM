cmake_minimum_required(VERSION 3.22)
project(macym VERSION 1.0.0 LANGUAGES C CXX)

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

juce_add_plugin(macym
    COMPANY_NAME "macYM"
    PLUGIN_MANUFACTURER_CODE McYm
    PLUGIN_CODE McY1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "macYM"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

target_sources(macym
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

# Vendored ayumi is a C library we don't modify; relax its per-file warnings.
set_source_files_properties(libs/ayumi/ayumi.c
    PROPERTIES COMPILE_OPTIONS "-Wno-missing-prototypes")

target_include_directories(macym
    PRIVATE
        libs/ayumi
        src
)

target_compile_definitions(macym
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(macym
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        juce::juce_recommended_lto_flags
)
