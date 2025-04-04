# Project setup
cmake_minimum_required(VERSION 3.15)
project(SimpleReverbProject VERSION 1.0.0)

# Add the original SimpleReverb standalone app with spectrum analyzer
add_executable(SimpleReverb SimpleReverb.cpp SpectrumAnalyzer.cpp)
target_compile_features(SimpleReverb PRIVATE cxx_std_14)
target_include_directories(SimpleReverb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific libraries
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

# Note: JUCE integration has been removed to simplify the build
# The project now focuses on the standalone SimpleReverb application
