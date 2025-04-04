# Custom Reverb Plugin

This is a professional C++ audio reverb effect implementation. The project contains three main components:

1. A SimpleReverb standalone application that processes WAV files
2. A text-based GUI for adjusting reverb parameters
3. A fully-featured JUCE-based audio plugin with graphical interface

## SimpleReverb Standalone Application

The SimpleReverb application implements a classic Schroeder reverb design that:

- Generates a test sine wave
- Applies configurable reverb effects
- Outputs both the original and processed audio as WAV files

### Features

- **Comb Filters**: Create initial reflections and echoes
- **Allpass Filters**: Diffuse the sound for smoother reverb
- **Frequency-Dependent Delay**: Create more natural reverb by delaying high frequencies differently
- **Adjustable Parameters**:
  - Room Size: Controls the size of the simulated space
  - Damping: Controls high frequency decay
  - Wet Level: Amount of processed signal
  - Dry Level: Amount of unprocessed signal  
  - Width: Stereo width control
  - Freeze Mode: For infinite reverb effects
  - High Frequency Delay: Adds natural delay to higher frequencies
  - Crossover: Sets the frequency threshold for high frequency processing

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

## JUCE Plugin

The project includes a fully-featured JUCE-based plugin that:

- Provides a professional graphical user interface
- Supports VST3, AU, and Standalone plugin formats
- Features preset system with 7 carefully designed reverb types
- Implements advanced frequency-dependent processing

### Preset System

The plugin includes carefully designed presets for various applications:

1. Small Room - Tight, subtle ambience for close-mic recordings
2. Medium Room - Balanced space for instruments and vocals
3. Large Hall - Expansive concert hall for orchestral or dramatic effects
4. Cathedral - Huge, ethereal reverb with long decay
5. Special FX - Creative, infinite reverb for sound design
6. Bright Chamber - Clear, articulate reverb with minimal high frequency delay
7. Dark Space - Warm, diffused reverb with pronounced high frequency delay

### Building the Plugin

The plugin can be built using CMake:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

### Structure

- `Source/`: Contains the JUCE plugin source files
- `JuceLibraryCode/`: JUCE framework integration files
- `CMakeLists.txt`: Build configuration for the entire project

## Technical Design

The reverb implementation is based on the classic Schroeder reverb design with:

- Parallel comb filters with feedback and damping
- Series allpass filters for diffusion
- Stereo processing with adjustable width
- Frequency-dependent delay for more natural sounding reverberation
- Crossover filtering to separate frequency bands for specialized processing
