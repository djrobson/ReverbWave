/*
  ==============================================================================

    JuceHeader.h - Simplified JUCE header for Phase 2 testing

    This header provides the essential JUCE modules needed for testing
    our refactored ReverbWave code without requiring full plugin framework.

  ==============================================================================
*/

#pragma once

// Core JUCE modules needed for testing
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Plugin-specific defines needed by PluginProcessor.cpp
#ifndef JucePlugin_Name
#define JucePlugin_Name "ReverbWave"
#endif

#ifndef JucePlugin_Build_VST3
#define JucePlugin_Build_VST3 1
#endif

#ifndef JucePlugin_Build_AU
#define JucePlugin_Build_AU 1
#endif

#ifndef JucePlugin_Build_Standalone
#define JucePlugin_Build_Standalone 1
#endif
