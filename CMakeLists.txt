# Project setup
cmake_minimum_required(VERSION 3.15)
project(SimpleReverbProject VERSION 1.0.0)

# Include the standalone applications
# ------------------------------------------------------------------------------

# Add the original SimpleReverb standalone app with spectrum analyzer
add_executable(SimpleReverb SimpleReverb.cpp SpectrumAnalyzer.cpp)
target_compile_features(SimpleReverb PRIVATE cxx_std_14)
target_include_directories(SimpleReverb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific libraries for SimpleReverb
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()

# Create simple GUI application
add_executable(ReverbGUI GUI/ReverbGUI.cpp)
target_compile_features(ReverbGUI PRIVATE cxx_std_14)

# Create a sample integration demo
add_executable(IntegrationDemo Integration.cpp)
target_compile_features(IntegrationDemo PRIVATE cxx_std_14)

# JUCE VST Plugin Configuration
# ------------------------------------------------------------------------------

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Configure the JUCE plugin options
juce_add_plugin(ReverbVST
    # Plugin Version Info
    VERSION 1.0.0
    PLUGIN_MANUFACTURER_CODE ReVb
    PLUGIN_CODE RevP
    
    # Plugin Format Information
    FORMATS VST3 Standalone
    PRODUCT_NAME "SimpleReverb VST"
    
    # Plugin Type
    VST3_CATEGORIES "Fx" "Reverb"
    
    # Plugin Information
    COMPANY_NAME "SimpleReverb"
    COMPANY_WEBSITE "https://example.com"
    COMPANY_EMAIL "info@example.com"
    
    # Source Files
    SOURCES
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        SpectrumAnalyzer.cpp
        SpectrumAnalyzerJUCE.cpp
)

# Configure VST Processing settings
target_compile_definitions(ReverbVST
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link the necessary JUCE modules to the plugin
target_link_libraries(ReverbVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set target properties for the plugin
set_target_properties(ReverbVST PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED TRUE
)

# Include paths for the plugin
target_include_directories(ReverbVST
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
