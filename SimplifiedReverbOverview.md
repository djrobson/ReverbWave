# SimpleReverb Project Overview

## Project Description

SimpleReverb is a standalone C++ audio processing application that applies reverb effects to audio files and provides real-time spectrum visualization. The application is designed to be cross-platform compatible, running natively on both Windows and Unix-like systems.

## Key Components

The project consists of three main executables:

1. **SimpleReverb**: The core reverb processing application with interactive spectrum analyzer
2. **ReverbGUI**: A text-based interface for adjusting reverb parameters
3. **IntegrationDemo**: A demonstration of how the reverb engine can be integrated with other audio systems

## Features

### Audio Processing

- Multiple reverb types and parameters
- Room size and damping controls
- Wet/dry level adjustments
- Frequency-dependent delay for upper harmonics
- Freeze mode for infinite reverb
- Cross-platform WAV file processing

### Real-Time Spectrum Analyzer

- FFT-based frequency analysis
- Three visualization modes:
  - Wave: Fluid wave animation with physics-based movement
  - Bars: Traditional spectrum analyzer bars
  - Particles: Particle system visualization
- Multiple color schemes (Blue, Green, Purple)
- Interactive keyboard controls

### Cross-Platform Support

- Native compatibility with Windows and Unix-like systems
- Platform-specific optimizations for terminal handling
- Consistent visualization across different operating systems
- CMake build system for easy compilation on any platform

## Technical Implementation

### Audio Engine

- Custom reverb algorithm with multiple reflection models
- Fast Fourier Transform (FFT) for frequency analysis
- Real-time audio buffer processing
- Sample rate conversion and format handling

### User Interface

- Terminal-based interface with ANSI color support
- Interactive keyboard controls
- Real-time parameter adjustment
- Cross-platform terminal handling

### Visualization

- Custom spectrum analyzer with fluid animations
- Physics-based wave movement
- Frequency-to-color mapping
- Responsive visualization updating

## Building and Running

### Linux/Unix

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./SimpleReverb
```

### Windows

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
Release\SimpleReverb.exe
```

## Project Evolution

The project started as a JUCE-based audio plugin but evolved into a standalone C++ application for simplicity and better cross-platform compatibility. The current implementation no longer depends on JUCE, making it lighter and easier to build across different platforms.

## Future Directions

- Add more reverb algorithms and parameters
- Implement real-time audio input/output
- Improve visualization with more complex animations
- Add presets system for quick parameter recall
- Create a fully-featured GUI using a lightweight framework
