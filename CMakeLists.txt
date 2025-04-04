# Project setup
cmake_minimum_required(VERSION 3.15)
project(CustomReverbPlugin VERSION 1.0.0)

# Include JUCE subdirectory
add_subdirectory(JUCE)

# Define the plugin formats to build - VST3 and AU are the most common
set(PLUGIN_FORMATS 
    VST3
    AU
)

# Define the plugin parameters
juce_add_plugin(CustomReverbPlugin
    VERSION 1.0.0
    PLUGIN_NAME "Custom Reverb"
    COMPANY_NAME "YourCompany"
    PRODUCT_NAME "Custom Reverb"
    FORMATS ${PLUGIN_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "Yrcm"
    PLUGIN_CODE "Crvb"
    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Set C++ standard
target_compile_features(CustomReverbPlugin PRIVATE cxx_std_14)

# Add source files
target_sources(CustomReverbPlugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Link against JUCE modules
target_link_libraries(CustomReverbPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set binary directory for output
set_target_properties(CustomReverbPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
