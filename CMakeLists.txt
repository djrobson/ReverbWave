# Project setup
cmake_minimum_required(VERSION 3.15)
# This is the internal name of the project and the name of JUCE's shared code target
# Note: This cannot have spaces (it may be 2024, but you can't have it all!)
# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs display)
set(PROJECT_NAME "ReverbWave")

# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs will display)
# You can also just have it be the same thing as PROJECT_NAME
# You may want to append the major version on the end of this (and PROJECT_NAME) ala:
#   set(PROJECT_NAME "MyPlugin_v${MAJOR_VERSION}")
# Doing so enables major versions to show up in IDEs and DAWs as separate plugins
# allowing you to change parameters and behavior without breaking existing user projects
set(PRODUCT_NAME "Reverb Wave")

# Change me! Used for the MacOS bundle name and Installers
set(COMPANY_NAME "Robson Enterprises")

# Change me! Used for the MacOS bundle identifier (and signing)
set(BUNDLE_ID "co.relabs.ReverbWave")

# Change me! Set the plugin formats you want built
# Valid choices: AAX Unity VST VST3 AU AUv3 Standalone
set(FORMATS Standalone AU VST3 AUv3)
project(${PROJECT_NAME} VERSION 1.0.0)

# JUCE VST Plugin Configuration
# ------------------------------------------------------------------------------

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Add any other modules you want modules here, before the juce_add_plugin call
#add_subdirectory (Source)

# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
    # Icons for the standalone app
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/generated-icon.png"

    # Change me!
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"

    # On MacOS, plugin is copied to /Users/yourname/Library/Audio/Plug-Ins/
    COPY_PLUGIN_AFTER_BUILD FALSE

    # Change me!
    # A four-character plugin id
    # First character MUST be uppercase for AU formats
    PLUGIN_MANUFACTURER_CODE Robs

    # Change me!
    # A unique four-character plugin id
    # Note: this must have at least one upper-case character
    PLUGIN_CODE R001
    FORMATS "${FORMATS}"

    # The name of your final executable
    # This is how it's listed in the DAW
    # This can be different from PROJECT_NAME and can have spaces!
    # You might want to use v${MAJOR_VERSION} here once you go to v2...
    PRODUCT_NAME "${PRODUCT_NAME}")

juce_generate_juce_header("${PROJECT_NAME}")

# This lets us use our code in both the JUCE targets and our Test target
# Without running into ODR violations
add_library(SharedCode INTERFACE)

# Manually list all .h and .cpp files for the plugin
# If you are like me, you'll use globs for your sanity.
# Just ensure you employ CONFIGURE_DEPENDS so the build system picks up changes
# If you want to appease the CMake gods and avoid globs, manually add files like so:
# set(SourceFiles Source/PluginEditor.h Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginProcessor.cpp)
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})

# This is where you can set preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
    INTERFACE

    # JUCE_WEB_BROWSER and JUCE_USE_CURL off by default
    JUCE_WEB_BROWSER=0  # If you set this to 1, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you set this to 1, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0

    # Uncomment if you are paying for a an Indie/Pro license or releasing under GPLv3
    # JUCE_DISPLAY_SPLASH_SCREEN=0

    # lets the app known if we're Debug or Release
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"

    # JucePlugin_Name is for some reason doesn't use the nicer PRODUCT_NAME
    PRODUCT_NAME_WITHOUT_VERSION="ReverbWave"
)

target_link_libraries(SharedCode
    INTERFACE
    juce_audio_utils
    juce_audio_processors
    juce_dsp
    juce_gui_basics
    juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
    
# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# ==============================================================================
# Testing Framework Configuration
# ==============================================================================

option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    message(STATUS "Building ReverbWave tests...")
    enable_testing()

    # Phase 1: Simple standalone test executable (no JUCE dependencies)
    add_executable(ReverbWaveTests
        Tests/SimpleTest.cpp
    )

    # No JUCE dependencies - pure C++ test
    target_compile_features(ReverbWaveTests PRIVATE cxx_std_17)

    # Set up test properties
    set_target_properties(ReverbWaveTests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    # Phase 2: Real JUCE integration test executable (simplified approach)
    add_executable(ReverbWavePhase2Tests
        Tests/Phase2-RealRefactoringTests.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SpectrumAnalyzer.cpp
        Source/SpectrumAnalyzerJUCE.cpp
        Source/harmonic_detuning.cpp
    )

    # Link to JUCE modules directly for testing
    target_link_libraries(ReverbWavePhase2Tests PRIVATE
        juce_audio_basics
        juce_audio_processors
        juce_core
        juce_data_structures
        juce_events
        juce_gui_basics
        juce_gui_extra
        juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

    target_compile_features(ReverbWavePhase2Tests PRIVATE cxx_std_17)

    # Add include paths for JUCE modules and test headers
    target_include_directories(ReverbWavePhase2Tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
    )

    # Add plugin defines needed by PluginProcessor.cpp
    target_compile_definitions(ReverbWavePhase2Tests PRIVATE
        JucePlugin_Build_VST3=1
        JucePlugin_Build_AU=1
        JucePlugin_Build_Standalone=1
        JucePlugin_Name="ReverbWave"
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        PRODUCT_NAME_WITHOUT_VERSION="ReverbWave"
    )

    # Set up Phase 2 test properties
    set_target_properties(ReverbWavePhase2Tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    # Add tests to CTest
    add_test(NAME ReverbWaveUnitTests COMMAND ReverbWaveTests)
    add_test(NAME ReverbWaveJUCEIntegrationTests COMMAND ReverbWavePhase2Tests)

    message(STATUS "Phase 1 tests configured: make ReverbWaveTests && ./ReverbWaveTests")
    message(STATUS "Phase 2 tests configured: make ReverbWavePhase2Tests && ./ReverbWavePhase2Tests")
endif()
