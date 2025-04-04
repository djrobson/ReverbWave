/*
  ==============================================================================

    CustomReverbEditor.cpp
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CustomReverbAudioProcessorEditor::CustomReverbAudioProcessorEditor (CustomReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up custom look and feel
    customLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    customLookAndFeel.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    customLookAndFeel.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    
    // Room Size Slider
    roomSizeSlider.setLookAndFeel(&customLookAndFeel);
    roomSizeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    roomSizeSlider.setTextValueSuffix("%");
    addAndMakeVisible(roomSizeSlider);
    
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    roomSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(roomSizeLabel);
    
    // Damping Slider
    dampingSlider.setLookAndFeel(&customLookAndFeel);
    dampingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    dampingSlider.setTextValueSuffix("%");
    addAndMakeVisible(dampingSlider);
    
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dampingLabel);
    
    // Wet Level Slider
    wetLevelSlider.setLookAndFeel(&customLookAndFeel);
    wetLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    wetLevelSlider.setTextValueSuffix("%");
    addAndMakeVisible(wetLevelSlider);
    
    wetLevelLabel.setText("Wet Level", juce::dontSendNotification);
    wetLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wetLevelLabel);
    
    // Dry Level Slider
    dryLevelSlider.setLookAndFeel(&customLookAndFeel);
    dryLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    dryLevelSlider.setTextValueSuffix("%");
    addAndMakeVisible(dryLevelSlider);
    
    dryLevelLabel.setText("Dry Level", juce::dontSendNotification);
    dryLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryLevelLabel);
    
    // Width Slider
    widthSlider.setLookAndFeel(&customLookAndFeel);
    widthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    widthSlider.setTextValueSuffix("%");
    addAndMakeVisible(widthSlider);
    
    widthLabel.setText("Width", juce::dontSendNotification);
    widthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(widthLabel);
    
    // Freeze Mode Button
    freezeModeButton.setButtonText("Freeze");
    freezeModeButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::lightblue);
    addAndMakeVisible(freezeModeButton);
    
    // Attach sliders to APVTS parameters
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "roomSize", roomSizeSlider);
    
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "damping", dampingSlider);
    
    wetLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "wetLevel", wetLevelSlider);
    
    dryLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "dryLevel", dryLevelSlider);
    
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "width", widthSlider);
    
    freezeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "freezeMode", freezeModeButton);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

CustomReverbAudioProcessorEditor::~CustomReverbAudioProcessorEditor()
{
    roomSizeSlider.setLookAndFeel(nullptr);
    dampingSlider.setLookAndFeel(nullptr);
    wetLevelSlider.setLookAndFeel(nullptr);
    dryLevelSlider.setLookAndFeel(nullptr);
    widthSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void CustomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background with a gradient
    g.fillAll(juce::Colours::black);
    
    // Draw a gradient background
    juce::ColourGradient gradient(juce::Colours::darkblue, 0, 0, 
                                 juce::Colours::black, getWidth(), getHeight(), 
                                 false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
    
    // Draw a plugin title
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Custom Reverb", getLocalBounds(), juce::Justification::centredTop, true);
    
    // Draw a border around the UI
    g.setColour(juce::Colours::lightblue.withAlpha(0.4f));
    g.drawRoundedRectangle(10.0f, 40.0f, getWidth() - 20.0f, getHeight() - 50.0f, 10.0f, 2.0f);
}

void CustomReverbAudioProcessorEditor::resized()
{
    // Layout the UI components
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40); // Space for title
    
    // Create two rows
    auto topRow = area.removeFromTop(area.getHeight() / 2);
    auto bottomRow = area;
    
    // First row: Room Size, Damping, Width
    auto roomSizeArea = topRow.removeFromLeft(topRow.getWidth() / 3);
    roomSizeLabel.setBounds(roomSizeArea.removeFromTop(20));
    roomSizeSlider.setBounds(roomSizeArea.reduced(10));
    
    auto dampingArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    dampingLabel.setBounds(dampingArea.removeFromTop(20));
    dampingSlider.setBounds(dampingArea.reduced(10));
    
    auto widthArea = topRow;
    widthLabel.setBounds(widthArea.removeFromTop(20));
    widthSlider.setBounds(widthArea.reduced(10));
    
    // Second row: Wet Level, Dry Level, Freeze Mode
    auto wetLevelArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 3);
    wetLevelLabel.setBounds(wetLevelArea.removeFromTop(20));
    wetLevelSlider.setBounds(wetLevelArea.reduced(10));
    
    auto dryLevelArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    dryLevelLabel.setBounds(dryLevelArea.removeFromTop(20));
    dryLevelSlider.setBounds(dryLevelArea.reduced(10));
    
    auto freezeArea = bottomRow;
    freezeModeButton.setBounds(freezeArea.withSizeKeepingCentre(100, 30));
}
