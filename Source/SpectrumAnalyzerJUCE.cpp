/*
  ==============================================================================

    SpectrumAnalyzerJUCE.cpp
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

// This file contains the JUCE-compatible implementation for the SpectrumAnalyzer component
// It replaces our custom FFT implementation with JUCE's built-in FFT

// SpectrumAnalyzerComponent Implementation
// This matches the implementation in PluginEditor.cpp, but is kept here separately
// to maintain compatibility with both JUCE plugin and standalone applications

namespace SpectrumAnalyzerAdapter {
    
    // Convert standalone spectrum analyzer format to JUCE format
    void convertSpectrum(const float* input, float* output, int size) {
        std::memcpy(output, input, size * sizeof(float));
    }
    
    // Apply a smoothing algorithm to the spectrum data
    void smoothSpectrum(float* data, float* prevData, const float* targetData, 
                         int size, float smoothingCoeff) {
        for (int i = 0; i < size; ++i) {
            prevData[i] = data[i];
            data[i] = prevData[i] + smoothingCoeff * (targetData[i] - prevData[i]);
        }
    }
    
    // Apply fluid dynamics simulation to create animated wave effect
    void applyFluidDynamics(float* points, float* velocities, const float* targets, 
                            int size, float tension, float damping, float spreadFactor) {
        // Create temporary buffer for spread forces
        std::vector<float> spreadForces(size, 0.0f);
        
        // Apply spring force towards target
        for (int i = 0; i < size; ++i) {
            float force = tension * (targets[i] - points[i]);
            velocities[i] += force;
            
            // Apply velocity to position
            points[i] += velocities[i] * 0.05f; // Animation speed
            
            // Apply damping
            velocities[i] *= damping;
        }
        
        // Calculate spread forces between adjacent points
        for (int i = 1; i < size - 1; ++i) {
            float leftDiff = points[i-1] - points[i];
            float rightDiff = points[i+1] - points[i];
            spreadForces[i] = spreadFactor * (leftDiff + rightDiff);
        }
        
        // Apply spread forces
        for (int i = 1; i < size - 1; ++i) {
            velocities[i] += spreadForces[i];
        }
    }
    
    // Generate initial colors for the visualization based on scheme
    void getColorsForScheme(int scheme, juce::Colour& color1, juce::Colour& color2) {
        switch (scheme) {
            case 0: // Blue/Cyan
                color1 = juce::Colours::blue;
                color2 = juce::Colours::cyan;
                break;
            
            case 1: // Purple/Pink
                color1 = juce::Colours::purple;
                color2 = juce::Colours::pink;
                break;
            
            case 2: // Green/Yellow
                color1 = juce::Colours::green;
                color2 = juce::Colours::yellow;
                break;
            
            default:
                color1 = juce::Colours::blue;
                color2 = juce::Colours::cyan;
        }
    }
    
    // Draw wave animation based on spectrum data
    void drawWaveAnimation(juce::Graphics& g, const float* wavePoints, int size, 
                           int width, int height, int colorScheme) {
        // Get colors for the scheme
        juce::Colour baseColour1, baseColour2;
        getColorsForScheme(colorScheme, baseColour1, baseColour2);
        
        // Create gradients for the wave
        juce::ColourGradient gradient(
            baseColour1.withAlpha(0.8f), 0, height, 
            baseColour2.withAlpha(0.3f), 0, 0, false);
        
        g.setGradientFill(gradient);
        
        // Create a path for the wave
        juce::Path wavePath;
        wavePath.startNewSubPath(0, height);
        
        for (int i = 0; i < size; ++i) {
            float x = width * i / static_cast<float>(size - 1);
            float y = height * (1.0f - wavePoints[i]);
            
            if (i == 0)
                wavePath.startNewSubPath(x, y);
            else
                wavePath.lineTo(x, y);
        }
        
        // Complete the wave path
        wavePath.lineTo(width, height);
        wavePath.closeSubPath();
        
        // Fill the wave
        g.fillPath(wavePath);
        
        // Draw wave outline
        g.setColour(baseColour1.brighter(0.2f));
        g.strokePath(wavePath, juce::PathStrokeType(1.0f));
    }
}
