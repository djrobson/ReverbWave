# Custom Reverb Plugin

This is a C++ audio reverb effect implementation. The project contains two main components:

1. A SimpleReverb standalone application that processes WAV files
2. A JUCE-based plugin structure (currently in development)

## SimpleReverb Standalone Application

The SimpleReverb application implements a classic Schroeder reverb design that:

- Generates a test sine wave
- Applies configurable reverb effects
- Outputs both the original and processed audio as WAV files

### Features

- **Comb Filters**: Create initial reflections and echoes
- **Allpass Filters**: Diffuse the sound for smoother reverb
- **Adjustable Parameters**:
  - Room Size: Controls the size of the simulated space
  - Damping: Controls high frequency decay
  - Wet Level: Amount of processed signal
  - Dry Level: Amount of unprocessed signal  
  - Width: Stereo width control
  - Freeze Mode: For infinite reverb effects

### Building and Running

To build and run the SimpleReverb application:

```bash
make
./SimpleReverb
```

This will generate:
- `original.wav` - A pure sine wave test signal
- `reverb_output.wav` - The same signal with reverb applied

## ReverbGUI 

A simple text-based interface for demonstrating reverb presets:

```bash
g++ -std=c++14 -o ReverbGUI GUI/ReverbGUI.cpp
./ReverbGUI
```

## JUCE Plugin (In Development)

The project also includes a JUCE-based plugin structure that, when completed, will:

- Provide a graphical user interface
- Support VST/AU/AAX plugin formats
- Offer additional reverb types and features

### Structure

- `Source/`: Contains the JUCE plugin source files
- `JuceLibraryCode/`: JUCE framework integration files
- `CMakeLists.txt`: Build configuration for the JUCE plugin

## Technical Design

The reverb implementation is based on the classic Schroeder reverb design with:

- Parallel comb filters with feedback and damping
- Series allpass filters for diffusion
- Stereo processing with adjustable width
