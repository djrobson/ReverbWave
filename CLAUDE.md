# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ReverbWave is a professional C++ audio reverb effect plugin built with the JUCE framework. It features a sophisticated reverb algorithm based on the Schroeder design with comb and allpass filters, real-time spectrum analysis, and harmonic detuning for enhanced stereo imaging.

## Build System

This project uses CMake as its primary build system:

### Basic Build Commands
```bash
# Configure and build (Release)
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# For Windows
cmake --build . --config Release

# Build specific targets
make ReverbWave                    # Main plugin target
make ReverbWave_VST3              # VST3 plugin only
make ReverbWave_Standalone        # Standalone app only  
make ReverbWave_AU                # Audio Unit (macOS)
```

### Output Locations
- VST3 Plugin: `build/ReverbWave_artefacts/VST3/`
- Standalone App: `build/ReverbWave_artefacts/Standalone/`
- AU Plugin: `build/ReverbWave_artefacts/AU/` (macOS)

## Project Architecture

### Core Components

**Source Directory Structure:**
- `Source/PluginProcessor.h/cpp` - Main audio processor (CustomReverbAudioProcessor)
- `Source/PluginEditor.h/cpp` - GUI editor for the plugin
- `Source/SpectrumAnalyzer.h/cpp` - FFT-based spectrum analysis and visualization
- `Source/SpectrumAnalyzerJUCE.cpp` - JUCE-specific spectrum analyzer integration
- `Source/harmonic_detuning.h/cpp` - Stereo enhancement via harmonic detuning

**Key Classes:**
- `CustomReverbAudioProcessor` - Main plugin processor, handles audio I/O and parameter management
- `SpectrumAnalyzerComponent` - Real-time spectrum visualization component
- `FFT` - Custom FFT implementation for frequency analysis
- `SpectrumAnalyzer` - Core spectrum analysis engine

### Plugin Configuration
- **Company**: Robson Enterprises
- **Bundle ID**: co.relabs.ReverbWave
- **Plugin Formats**: VST3, AU, Standalone, AUv3
- **Plugin Manufacturer Code**: Robs
- **Plugin Code**: R001

### Audio Processing Features
- Schroeder-based reverb with comb and allpass filters
- Variable room size, damping, and wet/dry controls
- High-frequency delay with adjustable crossover
- Harmonic detuning for stereo width enhancement
- Freeze mode for infinite reverb tails
- Built-in presets (Small Room, Large Hall, Cathedral, etc.)

### Spectrum Analyzer Features
- Real-time FFT analysis (20Hz-20kHz)
- Multiple visualization modes (Wave, Bars, Particles)
- Color scheme options (Blue, Green, Purple)
- Fluid animation system with physics modeling

## Development Notes

### JUCE Framework Integration
- Uses JUCE 7.x framework for audio plugin development
- CMake configuration handles JUCE dependency automatically
- Plugin supports all major DAW formats through JUCE

### Platform Support
- Primary development on macOS/Linux
- Windows support via CMake
- Cross-platform JUCE ensures broad compatibility

### Code Conventions
- C++14 standard minimum
- JUCE coding conventions followed
- Header guards use `#pragma once`
- Class documentation follows Doxygen style

### Important Build Notes
- Source files are automatically discovered via `GLOB_RECURSE` in CMakeLists.txt
- Both `Source/` and `source/` directories exist (case sensitivity handling)
- JUCE modules are linked via `SharedCode` interface library
- Plugin copying enabled for macOS development workflow