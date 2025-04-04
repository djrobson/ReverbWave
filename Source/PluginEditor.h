/*
  ==============================================================================

    PluginEditor.h
    Created: Custom Reverb Plugin
    Author:  Audio Plugin Developer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CustomReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CustomReverbAudioProcessorEditor (CustomReverbAudioProcessor&);
    ~CustomReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Reference to the processor object
    CustomReverbAudioProcessor& audioProcessor;
    
    // Sliders for the reverb parameters
    juce::Slider roomSizeSlider;
    juce::Slider dampingSlider;
    juce::Slider widthSlider;
    juce::Slider wetLevelSlider;
    juce::Slider dryLevelSlider;
    
    // Labels for the sliders
    juce::Label roomSizeLabel;
    juce::Label dampingLabel;
    juce::Label widthLabel;
    juce::Label wetLevelLabel;
    juce::Label dryLevelLabel;
    
    // Slider attachments to connect sliders to parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryLevelAttachment;
    
    // Setup slider with common properties
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessorEditor)
};
