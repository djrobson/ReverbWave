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
    void perform(std::complex<float>* data) {
        // Bit-reversal permutation
        int j = 0;
        for (int i = 0; i < size - 1; i++) {
            if (i < j) {
                std::swap(data[i], data[j]);
            }
            int m = size / 2;
            while (m <= j) {
                j -= m;
                m /= 2;
            }
            j += m;
        }
        
        // Cooley-Tukey decimation-in-time algorithm
        for (int stage = 1; stage <= order; stage++) {
            int m = 1 << stage;
            int m2 = m / 2;
            int step = size / m;
            
            for (int k = 0; k < size; k += m) {
                for (int j = 0; j < m2; j++) {
                    std::complex<float> t = twiddles[j * step] * data[k + j + m2];
                    std::complex<float> u = data[k + j];
                    data[k + j] = u + t;
                    data[k + j + m2] = u - t;
                }
            }
        }
    }
    
    // Calculate frequency spectrum (magnitude only)
    void calculateMagnitudeSpectrum(const float* input, float* output, int numSamples) {
        // Copy input data to complex array and apply window
        std::vector<std::complex<float>> fftData(size);
        
        // Apply Hann window
        for (int i = 0; i < numSamples && i < size; i++) {
            float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (numSamples - 1)));
            fftData[i] = std::complex<float>(input[i] * window, 0.0f);
        }
        
        // Zero padding if needed
        for (int i = numSamples; i < size; i++) {
            fftData[i] = std::complex<float>(0.0f, 0.0f);
        }
        
        // Perform FFT
        perform(fftData.data());
        
        // Calculate magnitude spectrum
        for (int i = 0; i < size / 2; i++) {
            float real = fftData[i].real();
            float imag = fftData[i].imag();
            output[i] = std::sqrt(real * real + imag * imag) / (size / 2);
        }
    }
    
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
    void pushSample(float sample) {
        if (fifoIndex == fftSize) {
            if (!nextFFTBlockReady) {
                // Copy data to FFT buffer
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        
        fifo[fifoIndex++] = sample;
    }
    
    // Process the FFT and update the spectrum display data
    void update() {
        if (nextFFTBlockReady) {
            // Calculate the spectrum
            fft.calculateMagnitudeSpectrum(fftData.data(), scopeData.data(), fftSize);
            
            // Update the target values for animation
            for (int i = 0; i < scopeSize; ++i) {
                // Apply a logarithmic frequency scale
                auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
                auto fftDataIndex = std::min(fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
                
                targetSpectrumValues[i] = scopeData[fftDataIndex];
            }
            
            nextFFTBlockReady = false;
        }
        
        // Smooth the spectrum values
        for (int i = 0; i < scopeSize; ++i) {
            previousSpectrumValues[i] = spectrumValues[i];
            spectrumValues[i] = previousSpectrumValues[i] + 
                               smoothingCoefficient * (targetSpectrumValues[i] - previousSpectrumValues[i]);
        }
        
        // Update animation
        updateAnimation();
    }
    
    // Draw the spectrum to a character buffer (for terminal display)
    void draw(std::vector<std::string>& buffer, int width, int height) {
        // Clear the buffer
        buffer.clear();
        buffer.resize(height, std::string(width, ' '));
        
        // Draw grid lines
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (i == height - 1 || j == 0) {
                    buffer[i][j] = '+';
                }
            }
        }
        
        // Draw frequency labels (simplified for terminal)
        const char* labels[] = {"100", "1k", "10k"};
        int labelPositions[] = {width / 6, width / 2, width * 5 / 6};
        
        for (int i = 0; i < 3; ++i) {
            int pos = labelPositions[i];
            if (pos < width - 4) {
                for (int j = 0; j < strlen(labels[i]); ++j) {
                    if (pos + j < width) {
                        buffer[height - 1][pos + j] = labels[i][j];
                    }
                }
            }
        }
        
        // Draw the spectrum based on animation mode
        if (animationMode == 0) {  // Wave mode
            drawWaveMode(buffer, width, height);
        } else if (animationMode == 1) {  // Bar mode
            drawBarMode(buffer, width, height);
        } else {  // Particle mode
            drawParticleMode(buffer, width, height);
        }
    }
    
    // Set the animation mode (0=Wave, 1=Bars, 2=Particles)
    void setAnimationMode(int mode) {
        animationMode = mode % 3;
    }
    
    // Set the color scheme (0=Blue/Cyan, 1=Purple/Pink, 2=Green/Yellow)
    void setColorScheme(int scheme) {
        colorScheme = scheme % 3;
    }
    
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
    void updateAnimation() {
        // Update wave targets based on spectrum values
        for (int i = 0; i < waveTargets.size(); ++i) {
            waveTargets[i] = spectrumValues[i];
        }
        
        // Apply spring forces to each point
        for (int i = 0; i < wavePoints.size(); ++i) {
            // Spring force towards target
            float force = tension * (waveTargets[i] - wavePoints[i]);
            waveVelocities[i] += force;
            
            // Apply velocity to position
            wavePoints[i] += waveVelocities[i] * animationSpeed;
            
            // Apply damping
            waveVelocities[i] *= damping;
        }
        
        // Apply spreading between adjacent points for fluid behavior
        std::vector<float> spreadForces(wavePoints.size(), 0.0f);
        
        for (int i = 1; i < wavePoints.size() - 1; ++i) {
            float leftDiff = wavePoints[i-1] - wavePoints[i];
            float rightDiff = wavePoints[i+1] - wavePoints[i];
            spreadForces[i] = spreadFactor * (leftDiff + rightDiff);
        }
        
        // Apply spread forces
        for (int i = 1; i < wavePoints.size() - 1; ++i) {
            waveVelocities[i] += spreadForces[i];
        }
    }
    
    // Draw wave animation mode to character buffer
    void drawWaveMode(std::vector<std::string>& buffer, int width, int height) {
        for (int i = 0; i < scopeSize; ++i) {
            int x = width * i / scopeSize;
            if (x < width) {
                int yWave = (height - 2) * (1.0f - wavePoints[i]);
                yWave = std::max(0, std::min(yWave, height - 2));
                
                // Draw wave point (using simple characters for terminal compatibility)
                buffer[yWave][x] = '#'; // Heavy character for wave crest
                
                // Fill area below wave for solid appearance
                for (int y = yWave + 1; y < height - 1; ++y) {
                    buffer[y][x] = '.'; // Light character for fill
                }
            }
        }
    }
    
    // Draw bar animation mode to character buffer
    void drawBarMode(std::vector<std::string>& buffer, int width, int height) {
        const int numBars = std::min(width - 1, (int)wavePoints.size());
        const int barWidth = 1;
        
        for (int i = 0; i < numBars; ++i) {
            int x = i + 1; // Start from column 1 (0 is border)
            if (x < width) {
                int barHeight = (int)((height - 2) * wavePoints[i * wavePoints.size() / numBars]);
                barHeight = std::max(0, std::min(barHeight, height - 2));
                
                // Draw the bar
                for (int y = height - 2; y > height - 2 - barHeight; --y) {
                    buffer[y][x] = '|'; // Simple character for bars
                }
            }
        }
    }
    
    // Draw particle animation mode to character buffer
    void drawParticleMode(std::vector<std::string>& buffer, int width, int height) {
        const int numPoints = std::min(width - 1, (int)wavePoints.size());
        
        for (int i = 0; i < numPoints; ++i) {
            float level = wavePoints[i * wavePoints.size() / numPoints];
            
            // Only draw particles for frequencies with some energy
            if (level > 0.05f) {
                // Number of particles depends on energy
                int numParticles = (int)(level * 5.0f);
                
                for (int j = 0; j < numParticles; ++j) {
                    int x = i + 1; // Start from column 1 (0 is border)
                    int y = height - 2 - (int)((height - 2) * level * (0.5f + 0.5f * random() / RAND_MAX));
                    
                    if (x < width && y >= 0 && y < height - 1) {
                        buffer[y][x] = '*'; // Asterisk for particles
                    }
                }
            }
        }
    }
    
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
