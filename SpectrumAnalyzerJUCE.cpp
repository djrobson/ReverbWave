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
