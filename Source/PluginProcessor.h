/*
  ==============================================================================

    PluginProcessor.h
    Created: Custom Reverb Plugin
    Author:  Audio Plugin Developer

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
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    // Reverb processor and parameters
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    
    // Parameter layout creation
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // AudioProcessorValueTreeState to handle plugin parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter IDs
    static const juce::String roomSizeID;
    static const juce::String dampingID;
    static const juce::String widthID;
    static const juce::String wetLevelID;
    static const juce::String dryLevelID;
    
    // Update the reverb parameters based on current parameter values
    void updateReverbParameters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessor)
};
