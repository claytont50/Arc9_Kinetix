cmake_minimum_required(VERSION 3.22)
project(Arc9_Kinetix VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Arc9_Kinetix
    COMPANY_NAME "Arc-9"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Arc9
    PLUGIN_CODE Kntx
    FORMATS VST3
    PRODUCT_NAME "Arc-9 Kinetix"
)

target_sources(Arc9_Kinetix PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
)

target_compile_definitions(Arc9_Kinetix PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Arc9_Kinetix PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
)
