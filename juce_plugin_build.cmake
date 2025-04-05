# JUCE Plugin Build Configuration
# This file handles the specific configuration needed for building the JUCE VST plugin

# Find all JUCE modules required for the plugin
set(JUCE_MODULES
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    juce_core
    juce_data_structures
    juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
)

# Configure VST plugin details
juce_add_plugin(ReverbVST
    # Plugin Version Info
    VERSION 1.0.0
    PLUGIN_MANUFACTURER_CODE ReVb
    PLUGIN_CODE RevP
    
    # Plugin Format Information
    FORMATS VST3 Standalone
    PRODUCT_NAME "ReverbWave"
    
    # Plugin Type
    VST3_CATEGORIES "Fx" "Reverb"
    
    # Plugin Information
    COMPANY_NAME "Robson Enterprises"
    COMPANY_WEBSITE "https://relab.co"
    COMPANY_EMAIL "drobson@relab.co"
    
    # Source Files
    SOURCES
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        SpectrumAnalyzer.cpp
        SpectrumAnalyzerJUCE.cpp
        harmonic_detuning.cpp
)

# Configure VST Processing settings
target_compile_definitions(ReverbVST
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

# Link all required JUCE modules
target_link_libraries(ReverbVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_data_structures
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set target properties
set_target_properties(ReverbVST PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED TRUE
)

# Include paths
target_include_directories(ReverbVST
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
