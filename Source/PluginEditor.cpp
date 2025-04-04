/*
  ==============================================================================

    PluginEditor.cpp
    Created: Custom Reverb Plugin
    Author:  Audio Plugin Developer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CustomReverbAudioProcessorEditor::CustomReverbAudioProcessorEditor (CustomReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the UI elements
    
    // Configure sliders and labels
    setupSlider(roomSizeSlider, roomSizeLabel, "Room Size");
    setupSlider(dampingSlider, dampingLabel, "Damping");
    setupSlider(widthSlider, widthLabel, "Width");
    setupSlider(wetLevelSlider, wetLevelLabel, "Wet Level");
    setupSlider(dryLevelSlider, dryLevelLabel, "Dry Level");
    
    // Create the parameter attachments
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), CustomReverbAudioProcessor::roomSizeID, roomSizeSlider);
    
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), CustomReverbAudioProcessor::dampingID, dampingSlider);
    
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), CustomReverbAudioProcessor::widthID, widthSlider);
    
    wetLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), CustomReverbAudioProcessor::wetLevelID, wetLevelSlider);
    
    dryLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), CustomReverbAudioProcessor::dryLevelID, dryLevelSlider);
    
    // Set the plugin window size
    setSize (400, 300);
}

CustomReverbAudioProcessorEditor::~CustomReverbAudioProcessorEditor()
{
}

void CustomReverbAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    // Configure the slider
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setRange(0.0, 1.0);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(slider);
    
    // Configure the label
    label.setText(labelText, juce::dontSendNotification);
    label.attachToComponent(&slider, true);
    addAndMakeVisible(label);
}

//==============================================================================
void CustomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colours::darkgrey);
    
    // Set up the title
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Custom Reverb", getLocalBounds(), juce::Justification::centredTop, true);
}

void CustomReverbAudioProcessorEditor::resized()
{
    // Define layout rectangle
    juce::Rectangle<int> area = getLocalBounds();
    
    // Reserve space for title
    area.removeFromTop(40);
    
    // Layout area for controls
    const int sliderHeight = 40;
    const int margin = 10;
    
    // Position the sliders
    area.removeFromLeft(80); // space for labels
    juce::Rectangle<int> sliderArea = area.reduced(margin);
    
    roomSizeSlider.setBounds(sliderArea.removeFromTop(sliderHeight));
    sliderArea.removeFromTop(margin);
    
    dampingSlider.setBounds(sliderArea.removeFromTop(sliderHeight));
    sliderArea.removeFromTop(margin);
    
    widthSlider.setBounds(sliderArea.removeFromTop(sliderHeight));
    sliderArea.removeFromTop(margin);
    
    wetLevelSlider.setBounds(sliderArea.removeFromTop(sliderHeight));
    sliderArea.removeFromTop(margin);
    
    dryLevelSlider.setBounds(sliderArea.removeFromTop(sliderHeight));
}
