# SimpleReverb: Cross-Platform Audio Reverb & Spectrum Analyzer

A professional C++ audio reverb effect implementation with real-time spectrum visualization. The project is designed to be cross-platform compatible, running natively on both Windows and Unix-like systems, with VST plugin support using the JUCE framework.

## Project Components

The project consists of four main components:

1. **SimpleReverb**: Core audio processor with spectrum analyzer visualization
2. **ReverbGUI**: Text-based interface for adjusting reverb parameters
3. **IntegrationDemo**: Demonstration of reverb integration with a plugin system
4. **ReverbVST**: VST3 and standalone audio plugin implementation using JUCE

## Features

### Audio Processing

- **Classic Schroeder Reverb Design**:
  - Comb filters for initial reflections and echoes
  - Allpass filters for sound diffusion
  - Frequency-dependent delay for natural reverberation
  
- **Advanced Reverb Parameters**:
  - Room Size: Controls the size of the simulated space
  - Damping: Controls high frequency decay
  - Wet/Dry Levels: Balance between processed and original signal
  - Width: Stereo width control
  - Freeze Mode: For infinite reverb effects
  - High Frequency Delay: Adds natural delay to higher frequencies
  - Crossover: Sets the frequency threshold for high frequency processing

### Real-Time Spectrum Analyzer

- **Interactive Visualization**:
  - FFT-based frequency analysis
  - Multiple animation modes (Wave, Bars, Particles)
  - Color scheme options (Blue, Green, Purple)
  - Keyboard controls for changing visualization settings

- **Test Signal Generation**:
  - Generates test signals with multiple frequencies (200, 400, 800, 1600, 3200, 6400 Hz)
  - Creates WAV files for both original and processed audio

### Cross-Platform Compatibility

- **Native Support For**:
  - Windows (Visual Studio, MinGW)
  - Linux/Unix systems
  - macOS (requires minor adjustments)

- **Platform-Specific Optimizations**:
  - Terminal color handling for all platforms
  - Non-blocking keyboard input
  - CMake build system with conditional linking

### VST Plugin Support

- **Plugin Formats**:
  - VST3: Compatible with major DAWs
  - Standalone Application: Run without a host

- **Plugin Features**:
  - GUI with spectrum analyzer visualization
  - Same high-quality reverb algorithm as standalone version
  - Parameter automation and state saving
  - Multiple presets

## Building and Running

### Prerequisites

- CMake (3.15 or higher)
- C++ compiler with C++14 support
- Basic development tools (make, etc.)

### Build Instructions

#### Linux/Unix

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

#### Windows

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Building the VST Plugin

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make ReverbVST
```

This will create:
- A VST3 plugin in `build/ReverbVST_artefacts/VST3/`
- A standalone application in `build/ReverbVST_artefacts/Standalone/`

### Running the Applications

#### SimpleReverb (with spectrum analyzer)

```bash
# Linux/Unix
./build/SimpleReverb

# Windows
.\build\Release\SimpleReverb.exe
```

#### ReverbGUI (parameter interface)

```bash
# Linux/Unix
./build/ReverbGUI

# Windows
.\build\Release\ReverbGUI.exe
```

#### IntegrationDemo

```bash
# Linux/Unix
./build/IntegrationDemo

# Windows
.\build\Release\IntegrationDemo.exe
```

#### ReverbVST (standalone plugin)

```bash
# Linux/Unix
./build/ReverbVST_artefacts/Standalone/SimpleReverb\ VST

# Windows
.\build\ReverbVST_artefacts\Standalone\SimpleReverb VST.exe
```

## Presets

The reverb engine includes carefully designed presets for various applications:

1. **Small Room**: Tight, subtle ambience for close-mic recordings
2. **Medium Room**: Balanced space for instruments and vocals
3. **Large Hall**: Expansive concert hall for orchestral or dramatic effects
4. **Cathedral**: Huge, ethereal reverb with long decay
5. **Special FX**: Creative, infinite reverb for sound design
6. **Bright Chamber**: Clear, articulate reverb with minimal high frequency delay
7. **Dark Space**: Warm, diffused reverb with pronounced high frequency delay

## Technical Implementation

The implementation includes:

- Custom FFT for frequency analysis
- Physics-based fluid wave animations
- Platform-specific optimizations for terminal handling
- Cross-platform keyboard input system
- CMake build system with automatic dependency resolution
- JUCE framework integration for VST plugin

## Project Structure

- `SimpleReverb.cpp`: Main implementation with reverb algorithm and visualization
- `SpectrumAnalyzer.h/cpp`: FFT analysis and fluid wave animations
- `GUI/ReverbGUI.cpp`: Text-based interface for reverb parameters
- `Integration.cpp`: Demonstration of plugin system integration
- `Source/PluginProcessor.h/cpp`: JUCE VST plugin processor implementation
- `Source/PluginEditor.h/cpp`: JUCE VST plugin editor implementation
- `CMakeLists.txt`: Build configuration for cross-platform compatibility

## Documentation

Additional documentation is available in the project files:

- `SimplifiedReverbOverview.md`: Concise project overview
- `CrossPlatformCompatibility.md`: Details on cross-platform support
- `WindowsCompatibility.md`: Windows-specific implementation notes
- `WindowsCompatibilityImplementation.md`: Detailed implementation guide for Windows
- `JUCE_Integration_Notes.md`: Notes on JUCE integration for VST plugin

## JUCE Integration

The project supports JUCE integration for VST plugin creation. The JUCE framework is used as a submodule to provide VST3 and standalone formats, while maintaining compatibility with the original standalone applications.
