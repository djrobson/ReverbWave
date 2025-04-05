# Project setup
cmake_minimum_required(VERSION 3.15)
project(ReverbWave VERSION 1.0.0)

# Include the standalone applications
# ------------------------------------------------------------------------------

# Add the original SimpleReverb standalone app with spectrum analyzer
add_executable(SimpleReverb SimpleReverb.cpp SpectrumAnalyzer.cpp harmonic_detuning.cpp)
target_compile_features(SimpleReverb PRIVATE cxx_std_14)
target_include_directories(SimpleReverb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific libraries for SimpleReverb
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()

# Create simple GUI application
add_executable(ReverbGUI GUI/ReverbGUI.cpp harmonic_detuning.cpp)
target_compile_features(ReverbGUI PRIVATE cxx_std_14)
target_include_directories(ReverbGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# JUCE VST Plugin Configuration
# ------------------------------------------------------------------------------

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Include the JUCE plugin build file
include(juce_plugin_build.cmake)
