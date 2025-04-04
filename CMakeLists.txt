# Project setup
cmake_minimum_required(VERSION 3.15)
project(CustomReverbPlugin VERSION 1.0.0)

# Add the original SimpleReverb standalone app with spectrum analyzer
add_executable(SimpleReverb SimpleReverb.cpp)
target_compile_features(SimpleReverb PRIVATE cxx_std_14)
target_include_directories(SimpleReverb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SimpleReverb PRIVATE m pthread)

# Create simple GUI application
add_executable(ReverbGUI GUI/ReverbGUI.cpp)
target_compile_features(ReverbGUI PRIVATE cxx_std_14)

# Create a sample integration demo
add_executable(IntegrationDemo Integration.cpp)
target_compile_features(IntegrationDemo PRIVATE cxx_std_14)

# JUCE Plugin Setup
# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Initialize JUCE project
juce_add_plugin(CustomReverbPlugin
    COMPANY_NAME "Audio Developer"
    PLUGIN_MANUFACTURER_CODE Adpt
    PLUGIN_CODE Crvb
    FORMATS Standalone
    PRODUCT_NAME "Custom Reverb")

# Specify the plugin sources
target_sources(CustomReverbPlugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

# Set C++ standard
target_compile_features(CustomReverbPlugin PRIVATE cxx_std_14)

# Add required JUCE modules
target_link_libraries(CustomReverbPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Add include directories
target_include_directories(CustomReverbPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
        
# Generate the JuceHeader file
juce_generate_juce_header(CustomReverbPlugin)
