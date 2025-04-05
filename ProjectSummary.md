# ReverbVST Project Summary

## Project Overview
This project implements a feature-rich audio reverb effect with multiple implementations:

1. A standalone C++ processor for WAV files with real-time spectrum analysis and fluid wave animations
2. A terminal-based GUI for controlling reverb parameters
3. A VST3 plugin version compatible with digital audio workstations (DAWs)

## Components

### ReverbVST (Plugin Version)
- Complete VST3 plugin using the JUCE framework
- Integrates the reverb algorithm into a professional plugin format
- Includes spectrum analysis visualization
- Compatible with all major DAWs supporting VST3

## Reverb Algorithm Features
- Combines comb and allpass filters for natural reverb sound
- Separate high and low frequency processing paths
- Adjustable crossover frequency
- Variable high-frequency delay for greater sound depth
- Stereo width control for spatial enhancement
- Harmonic detuning for enhanced stereo image and tonal complexity
- Freeze mode for infinite reverb tails

## Spectrum Analyzer Features
- Real-time FFT-based frequency analysis
- Multiple visualization modes (Wave, Bars)
- Multiple color schemes
- Fluid animations for dynamic visual feedback
- Logarithmic frequency scale (20Hz to 20kHz)

## Project Architecture
- Core reverb algorithm implemented in C++ (SimpleReverb class)
- Modularity through clear class separation:
  - SimpleReverb: Core algorithm
  - SimpleAudioProcessor: Processing chain and integration
  - SpectrumAnalyzer: Audio analysis and visualization
  - ReverbGUI: User interface
- JUCE integration for plugin functionality

## Build Information
- CMake-based build system for cross-platform compatibility
- Multiple executable targets:
  - ReverbVST (VST3 plugin)
- Support for Linux and Windows platforms

## Usage Instructions
- **ReverbVST**: Use as a plugin in compatible DAWs
  - Copy the VST3 plugin from `./build/ReverbVST_artefacts/Release/`
  - Place in your DAW's plugin directory