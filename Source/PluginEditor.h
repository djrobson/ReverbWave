/*
  ==============================================================================

    CustomReverbEditor.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Spectrum Analyzer Component with Fluid Wave Animations
 * Displays a real-time FFT of the audio output with an animated fluid wave visualization
 */
class SpectrumAnalyzerComponent : public juce::Component,
                                 private juce::Timer
{
public:
    SpectrumAnalyzerComponent(CustomReverbAudioProcessor& processor);
    ~SpectrumAnalyzerComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Called from the processor to update the spectrum data
    void updateSpectrum(const float* spectrumData, int numBins);
    
    // Control the animation style
    void setAnimationMode(int mode);
    void setColorScheme(int scheme);
    
private:
    void timerCallback() override;
    
    // Animation parameters
    void updateAnimation();
    
    CustomReverbAudioProcessor& processorRef;
    
    // FFT data and display
    juce::dsp::FFT fft;
    juce::HeapBlock<float> fftData;
    int fftSize;
    
    // Spectrum data buffers
    std::vector<float> spectrumValues;
    std::vector<float> previousSpectrumValues;
    std::vector<float> targetSpectrumValues;
    
    // Wave animation data
    std::vector<float> wavePoints;
    std::vector<float> waveVelocities;
    std::vector<float> waveTargets;
    
    // Animation properties
    float smoothingCoefficient = 0.2f;
    float animationSpeed = 0.05f;
    int animationMode = 0; // 0=Wave, 1=Bars, 2=Particles
    int colorScheme = 0;   // 0=Blue/Cyan, 1=Purple/Pink, 2=Green/Yellow
    bool useGradient = true;
    
    // Fluid dynamics parameters
    float damping = 0.97f;
    float tension = 0.025f;
    float spreadFactor = 0.2f;
    
    juce::Random random;
    juce::Colour baseColour1 = juce::Colours::blue;
    juce::Colour baseColour2 = juce::Colours::cyan;
};

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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CustomReverbAudioProcessor& audioProcessor;
    
    // UI Components
    juce::Slider roomSizeSlider;
    juce::Slider dampingSlider;
    juce::Slider wetLevelSlider;
    juce::Slider dryLevelSlider;
    juce::Slider widthSlider;
    juce::Slider highFreqDelaySlider;
    juce::Slider crossoverSlider;
    juce::Slider harmDetuneAmountSlider;
    juce::ToggleButton freezeModeButton;
    juce::ComboBox presetSelector;
    
    // Spectrum analyzer component
    SpectrumAnalyzerComponent spectrumAnalyzer;
    juce::TextButton animationStyleButton;
    juce::TextButton colorSchemeButton;
    
    // Labels for sliders
    juce::Label roomSizeLabel;
    juce::Label dampingLabel;
    juce::Label wetLevelLabel;
    juce::Label dryLevelLabel;
    juce::Label widthLabel;
    juce::Label highFreqDelayLabel;
    juce::Label crossoverLabel;
    juce::Label presetLabel;
    juce::Label spectrumLabel;
    
    // Slider attachment objects to connect the UI to the parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highFreqDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeModeAttachment;
    
    // Custom LookAndFeel for the sliders
    juce::LookAndFeel_V4 customLookAndFeel;
    
    // Preset handling methods
    void setupPresetMenu();
    void loadPreset(int presetIndex);
    
    // Animation/visualization style methods
    void cycleAnimationStyle();
    void cycleColorScheme();
    int currentAnimationStyle = 0;
    int currentColorScheme = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessorEditor)
};
