/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
  
  This file contains the implementation of a real-time audio spectrum analyzer
  with interactive fluid animations. It provides multiple visualization modes
  and color schemes for representing audio frequency content.
  
  Key features:
  - Fast Fourier Transform (FFT) implementation for frequency analysis
  - Fluid wave animation using simplified physics modeling
  - Multiple visualization styles (waves, bars, particles)
  - Smooth transitions between spectrum frames
  - Terminal-based rendering for non-graphical environments
  - Color scheme options for different aesthetic preferences
  
  The SpectrumAnalyzer can be used both in standalone applications and
  as part of a VST/AU plugin interface to visualize the frequency content
  of audio being processed.
*/

#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <functional>
#include <random>
#include <memory>
#include <complex>
#include <cstring>

// Define M_PI for Windows if it's not defined
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

/**
 * FFT (Fast Fourier Transform) Implementation
 * 
 * A lightweight FFT implementation for real-time audio spectrum analysis.
 * This class handles the conversion from time-domain audio samples to
 * frequency-domain spectral data for visualization.
 */
class FFT {
public:
    /**
     * Construct an FFT processor with the specified order
     * @param order The FFT order (size will be 2^order)
     */
    FFT(int order) : order(order), size(1 << order) {
        // Pre-compute twiddle factors
        twiddles.resize(size / 2);
        for (int i = 0; i < size / 2; i++) {
            double angle = -2.0 * M_PI * i / size;
            twiddles[i] = std::complex<float>(cos(angle), sin(angle));
        }
    }
    
    /**
     * Perform in-place FFT on complex data
     * @param data Complex data array to transform
     */
    void perform(std::complex<float>* data);
    
    /**
     * Calculate magnitude spectrum from time-domain input
     * @param input Input audio samples (time domain)
     * @param output Output magnitude spectrum (frequency domain)
     * @param numSamples Number of samples to process
     */
    void calculateMagnitudeSpectrum(const float* input, float* output, int numSamples);
    
    /**
     * Get the FFT size
     * @return The size of the FFT (2^order)
     */
    int getSize() const { return size; }
    
private:
    int order;                              // FFT order (log2 of size)
    int size;                               // FFT size (2^order)
    std::vector<std::complex<float>> twiddles;  // Pre-computed twiddle factors
};

// Forward declaration
class SimpleAudioProcessor;

/**
 * SpectrumAnalyzer Class
 * 
 * Provides real-time audio spectrum analysis with fluid wave animations.
 * This class handles the audio processing, FFT calculation, and visualization
 * rendering for audio frequency content.
 */
class SpectrumAnalyzer {
public:
    /**
     * Constructor - initializes the spectrum analyzer with default settings
     */
    SpectrumAnalyzer() : fft(11), random(std::random_device()()) {
        fftSize = 1 << 11;  // 2048 samples
        scopeSize = 512;    // Display resolution
        
        // Initialize spectrum data buffers
        spectrumValues.resize(scopeSize, 0.0f);
        previousSpectrumValues.resize(scopeSize, 0.0f);
        targetSpectrumValues.resize(scopeSize, 0.0f);
        fftData.resize(fftSize, 0.0f);
        scopeData.resize(scopeSize, 0.0f);
        
        // Initialize wave animation data
        wavePoints.resize(scopeSize, 0.0f);
        waveVelocities.resize(scopeSize, 0.0f);
        waveTargets.resize(scopeSize, 0.0f);
        
        // Set default animation properties
        smoothingCoefficient = 0.2f;
        animationSpeed = 0.05f;
        animationMode = 0;  // Wave mode
        colorScheme = 0;    // Blue/Cyan
        
        // Set fluid dynamics parameters
        damping = 0.97f;
        tension = 0.025f;
        spreadFactor = 0.2f;
        
        // Set up FIFO for input data
        fifo.resize(fftSize, 0.0f);
        fifoIndex = 0;
        nextFFTBlockReady = false;
    }
    
    /**
     * Push a mono sample into the FFT FIFO
     * @param sample Audio sample to analyze
     */
    void pushSample(float sample);
    
    /**
     * Process the FFT and update the spectrum display data
     * Called periodically to refresh the visualization
     */
    void update();
    
    /**
     * Draw the spectrum to a character buffer (for terminal display)
     * @param buffer The string buffer to render to
     * @param width Display width in characters
     * @param height Display height in characters
     */
    void draw(std::vector<std::string>& buffer, int width, int height);
    
    /**
     * Set the animation mode
     * @param mode Animation mode (0=Wave, 1=Bars, 2=Particles)
     */
    void setAnimationMode(int mode);
    
    /**
     * Set the color scheme
     * @param scheme Color scheme (0=Blue/Cyan, 1=Purple/Pink, 2=Green/Yellow)
     */
    void setColorScheme(int scheme);
    
    /**
     * Get current animation mode
     * @return Current animation mode
     */
    int getAnimationMode() const {
        return animationMode;
    }
    
    /**
     * Get current color scheme
     * @return Current color scheme
     */
    int getColorScheme() const {
        return colorScheme;
    }
    
private:
    /**
     * Update the fluid wave animation
     * Uses simplified physics to create natural-looking wave motion
     */
    void updateAnimation();
    
    /**
     * Draw wave animation mode to character buffer
     */
    void drawWaveMode(std::vector<std::string>& buffer, int width, int height);
    
    /**
     * Draw bar animation mode to character buffer
     */
    void drawBarMode(std::vector<std::string>& buffer, int width, int height);
    
    /**
     * Draw particle animation mode to character buffer
     */
    void drawParticleMode(std::vector<std::string>& buffer, int width, int height);
    
    // FFT implementation
    FFT fft;                    // FFT processor
    int fftSize;                // FFT size
    int scopeSize;              // Display resolution
    
    // Spectrum data
    std::vector<float> spectrumValues;          // Current spectrum values
    std::vector<float> previousSpectrumValues;  // Previous frame values for smoothing
    std::vector<float> targetSpectrumValues;    // Target values for animation
    std::vector<float> fftData;                 // Raw FFT data
    std::vector<float> scopeData;               // Processed spectrum data
    
    // Wave animation data
    std::vector<float> wavePoints;              // Current wave positions
    std::vector<float> waveVelocities;          // Wave movement velocities
    std::vector<float> waveTargets;             // Target positions for waves
    
    // Animation properties
    float smoothingCoefficient;     // Controls smoothness between frames
    float animationSpeed;           // Speed of animations
    int animationMode;              // Current animation mode
    int colorScheme;                // Current color scheme
    
    // Fluid dynamics parameters
    float damping;                  // How quickly motion slows down
    float tension;                  // How strongly points pull toward targets
    float spreadFactor;             // How much adjacent points influence each other
    
    // Input FIFO
    std::vector<float> fifo;        // FIFO buffer for input samples
    int fifoIndex;                  // Current position in FIFO
    bool nextFFTBlockReady;         // Flag to indicate FFT can be performed
    
    // Random number generator
    std::mt19937 random;            // Random number generator for particle effects
};
