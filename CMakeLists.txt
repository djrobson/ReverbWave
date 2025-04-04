# Project setup
cmake_minimum_required(VERSION 3.15)
project(CustomReverbPlugin VERSION 1.0.0)

# Since we're having issues with the JUCE integration, let's modify to build
# our standalone SimpleReverb application cleanly without JUCE dependencies

add_executable(SimpleReverb SimpleReverb.cpp)
target_compile_features(SimpleReverb PRIVATE cxx_std_14)
target_link_libraries(SimpleReverb PRIVATE m)

# Create simple GUI application
add_executable(ReverbGUI GUI/ReverbGUI.cpp)
target_compile_features(ReverbGUI PRIVATE cxx_std_14)

# Create a sample integration demo
add_executable(IntegrationDemo Integration.cpp)
target_compile_features(IntegrationDemo PRIVATE cxx_std_14)
