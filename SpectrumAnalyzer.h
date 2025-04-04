/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
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

// Simple FFT implementation for audio spectrum analysis
class FFT {
public:
    FFT(int order) : order(order), size(1 << order) {
        // Pre-compute twiddle factors
        twiddles.resize(size / 2);
        for (int i = 0; i < size / 2; i++) {
            double angle = -2.0 * M_PI * i / size;
            twiddles[i] = std::complex<float>(cos(angle), sin(angle));
        }
    }
    
    // Perform in-place FFT
    void perform(std::complex<float>* data);
    
    // Calculate frequency spectrum (magnitude only)
    void calculateMagnitudeSpectrum(const float* input, float* output, int numSamples);
    
    int getSize() const { return size; }
    
private:
    int order;
    int size;
    std::vector<std::complex<float>> twiddles;
};

// Forward declaration
class SimpleAudioProcessor;

// SpectrumAnalyzer class
class SpectrumAnalyzer {
public:
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
    
    // Push a mono sample into the FFT FIFO
    void pushSample(float sample);
    
    // Process the FFT and update the spectrum display data
    void update();
    
    // Draw the spectrum to a character buffer (for terminal display)
    void draw(std::vector<std::string>& buffer, int width, int height);
    
    // Set the animation mode (0=Wave, 1=Bars, 2=Particles)
    void setAnimationMode(int mode);
    
    // Set the color scheme (0=Blue/Cyan, 1=Purple/Pink, 2=Green/Yellow)
    void setColorScheme(int scheme);
    
    // Get current animation mode
    int getAnimationMode() const {
        return animationMode;
    }
    
    // Get current color scheme
    int getColorScheme() const {
        return colorScheme;
    }
    
private:
    // Update the fluid wave animation
    void updateAnimation();
    
    // Draw wave animation mode to character buffer
    void drawWaveMode(std::vector<std::string>& buffer, int width, int height);
    
    // Draw bar animation mode to character buffer
    void drawBarMode(std::vector<std::string>& buffer, int width, int height);
    
    // Draw particle animation mode to character buffer
    void drawParticleMode(std::vector<std::string>& buffer, int width, int height);
    
    // FFT implementation
    FFT fft;
    int fftSize;
    int scopeSize;
    
    // Spectrum data
    std::vector<float> spectrumValues;
    std::vector<float> previousSpectrumValues;
    std::vector<float> targetSpectrumValues;
    std::vector<float> fftData;
    std::vector<float> scopeData;
    
    // Wave animation data
    std::vector<float> wavePoints;
    std::vector<float> waveVelocities;
    std::vector<float> waveTargets;
    
    // Animation properties
    float smoothingCoefficient;
    float animationSpeed;
    int animationMode;
    int colorScheme;
    
    // Fluid dynamics parameters
    float damping;
    float tension;
    float spreadFactor;
    
    // Input FIFO
    std::vector<float> fifo;
    int fifoIndex;
    bool nextFFTBlockReady;
    
    // Random number generator
    std::mt19937 random;
};
