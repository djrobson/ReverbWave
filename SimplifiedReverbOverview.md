# Custom Reverb Plugin

This is a professional C++ implementation of a versatile reverb audio effect. The project is organized into several components:

1. A standalone SimpleReverb application for processing WAV files
2. A text-based GUI interface for adjusting reverb parameters
3. A JUCE-based VST/AU plugin with a professional graphical interface

## Features

### Basic Parameters
- Room size control: Adjusts the size of the simulated space
- Damping control: Adjusts high-frequency absorption
- Wet/dry levels: Controls mix between processed and original sound
- Width control: Adjusts stereo width of the reverb effect
- Freeze mode: Creates infinite sustain effect

### Advanced Parameters
- High Frequency Delay: Creates a more natural reverb by delaying higher frequencies
- Crossover Frequency: Controls which frequencies are affected by the high frequency delay

### Additional Features
- Preset system with various reverb types:
  - Small Room
  - Medium Room
  - Large Hall
  - Cathedral
  - Special FX
  - Bright Chamber
  - Dark Space
- Professional GUI with visual feedback for all parameters
- Real-Time Spectrum Analyzer with Fluid Wave Animations
  - Multiple visualization modes: Wave, Bars, and Particles
  - Customizable color schemes
  - Physics-based fluid animations for smooth, organic movement

## Architecture

### Standalone Applications
- SimpleReverb.cpp: Main implementation, processes WAV files with the reverb algorithm
- GUI/ReverbGUI.cpp: Simple text-based interface for adjusting parameters
- Integration.cpp: Demo showing integration with other audio systems

### JUCE Plugin Components
- Source/PluginProcessor.cpp: The audio processing implementation for the plugin
- Source/PluginEditor.cpp: The GUI implementation for the plugin
- JuceLibraryCode: JUCE framework integration files

## Technical Implementation Details

### High Frequency Processing
The reverb implements a frequency-dependent delay for upper harmonics which creates a more natural reverb sound. This simulates the physical phenomenon where high frequencies reflect differently than low frequencies in real acoustic spaces.

The implementation works by:
1. Splitting the input signal into low and high frequency components using a crossover filter
2. Applying a configurable delay to just the high frequency components
3. Recombining the signals before passing through the main reverb algorithm

### Preset System
The plugin includes a set of carefully designed presets that demonstrate different reverb characteristics:
- Bright Chamber: Less damping of high frequencies with minimal high frequency delay
- Dark Space: Heavy damping with significant high frequency delay
- Cathedral: Large room size with moderate high frequency characteristics

Each preset configures all parameters to create a cohesive reverb character suitable for different sound sources.

## Real-Time Spectrum Analyzer with Fluid Wave Animations

The plugin includes an advanced real-time spectrum analyzer that provides detailed visual feedback about the processed audio. Unlike traditional spectrum analyzers, this implementation features physics-based fluid animations that create organic, flowing visualizations that respond naturally to the audio content.

### Technical Implementation

The spectrum analyzer is built with the following components:

1. **Fast Fourier Transform (FFT) Processing:** 
   - Custom FFT implementation for frequency analysis
   - 2048-point FFT with windowing for accurate spectral representation
   - Logarithmic frequency mapping for more natural display

2. **Fluid Physics Simulation:**
   - Spring-mass model for each frequency band
   - Force-based interactions between adjacent bands
   - Configurable damping, tension, and spread factors
   - Velocity-based movement and smooth interpolation

3. **Multiple Visualization Styles:**
   - Wave Mode: A continuous wave with fluid movement
   - Bar Mode: Traditional spectrum bars with smooth animation
   - Particle Mode: Dynamic particle system responding to frequency content

4. **Real-Time Performance:**
   - Efficient implementation for minimal CPU usage
   - Automatic adjustment to varying buffer sizes
   - Smooth 60fps animation even during heavy processing

This visualization component enhances the user experience by providing immediate visual feedback on how the reverb is affecting different frequency ranges of the audio signal, making it easier to fine-tune parameters for optimal results.
