/*
  ==============================================================================

    CustomReverbProcessor.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CustomReverbAudioProcessor  : public juce::AudioProcessor,
                                   public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    CustomReverbAudioProcessor();
    ~CustomReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Parameter listener implementation
    void parameterChanged(const juce::String &parameterID, float newValue) override;
    
    // Reference to the APVTS - public for editor access
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    //==============================================================================
    // Reverb processor and parameters
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    
    // Audio processor value tree state for parameter management
    juce::AudioProcessorValueTreeState apvts;
    
    // Parameter creation helper
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    // Update reverb parameters based on current values
    void updateReverbParameters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessor)
};
