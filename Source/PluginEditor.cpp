/*
  ==============================================================================

    CustomReverbEditor.cpp
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
  
  This file implements the GUI components for the reverb plugin:
  
  1. SpectrumAnalyzerComponent 
     - Real-time frequency visualization with multiple animation styles
     - Physics-based fluid motion for smooth wave animations
     - Color scheme options and different rendering styles
     
  2. CustomReverbAudioProcessorEditor
     - Main plugin interface with interactive controls
     - Parameter sliders with custom styling and tooltips
     - Preset management system for quick parameter recall
     - Integration with the SpectrumAnalyzer for visual feedback
     
  The implementation connects UI controls to the AudioProcessorValueTreeState
  in the processor for automatic parameter synchronization and persistence.
  All visualizations update in real-time as the audio is processed.
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// SpectrumAnalyzerComponent Implementation
//==============================================================================

SpectrumAnalyzerComponent::SpectrumAnalyzerComponent(CustomReverbAudioProcessor& processor) 
    : processorRef(processor),
      fft(processor.fftOrder),
      fftSize(1 << processor.fftOrder)
{
    // Set up the FFT data array
    fftData.allocate (2 * fftSize, true);
    
    // Initialize spectrum data buffers
    spectrumValues.resize(processor.scopeSize, 0.0f);
    previousSpectrumValues.resize(processor.scopeSize, 0.0f);
    targetSpectrumValues.resize(processor.scopeSize, 0.0f);
    
    // Initialize wave animation data
    wavePoints.resize(processor.scopeSize, 0.0f);
    waveVelocities.resize(processor.scopeSize, 0.0f);
    waveTargets.resize(processor.scopeSize, 0.0f);
    
    // Register this component with the processor
    processor.setSpectrumAnalyzer(this);
    
    // Start the animation timer
    startTimerHz(60); // 60fps for smooth animation
}

SpectrumAnalyzerComponent::~SpectrumAnalyzerComponent()
{
    // Unregister from processor
    processorRef.setSpectrumAnalyzer(nullptr);
    stopTimer();
}

void SpectrumAnalyzerComponent::paint(juce::Graphics& g)
{
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());
    
    // Draw background
    g.fillAll(juce::Colour(10, 15, 20));
    
    // Draw grid lines
    g.setColour(juce::Colour(40, 45, 50));
    for (int i = 1; i < 10; ++i)
    {
        auto y = height * i / 10.0f;
        g.drawLine(0, y, width, y, 0.5f);
    }
    
    for (int i = 1; i < 10; ++i)
    {
        auto x = width * i / 10.0f;
        g.drawLine(x, 0, x, height, 0.5f);
    }
    
    // Draw frequency labels
    g.setColour(juce::Colours::grey);
    g.setFont(12.0f);
    
    auto sampleRate = processorRef.getSampleRate();
    if (sampleRate > 0)
    {
        const char* freqLabels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };
        const float freqValues[] = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
        
        for (int i = 0; i < 10; ++i)
        {
            auto freq = freqValues[i];
            auto normX = std::log10(freq / 20.0f) / std::log10(20000.0f / 20.0f);
            auto x = width * normX;
            
            if (x >= 0 && x < width)
            {
                g.drawText(freqLabels[i], static_cast<int>(x) - 10, (int)height - 20, 20, 20, juce::Justification::centred);
                g.drawLine(x, height - 22, x, height - 18, 1.0f);
            }
        }
    }
    
    // Draw the fluid wave animation based on spectrum data
    if (animationMode == 0) // Wave mode
    {
        // Create gradients for the wave
        juce::ColourGradient gradient;
        
        if (useGradient)
        {
            gradient = juce::ColourGradient(baseColour1.withAlpha(0.8f), 0, height, 
                                           baseColour2.withAlpha(0.3f), 0, 0, false);
        }
        else
        {
            gradient = juce::ColourGradient(baseColour1.withAlpha(0.8f), 0, height, 
                                           baseColour1.withAlpha(0.3f), 0, 0, false);
        }
        
        g.setGradientFill(gradient);
        
        // Create a path for the wave
        juce::Path wavePath;
        wavePath.startNewSubPath(0, height);
        
        for (int i = 0; i < spectrumValues.size(); ++i)
        {
            float x = width * i / static_cast<float>(spectrumValues.size() - 1);
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
        
        // Add some particle effects for extra flair
        g.setColour(baseColour2.brighter(0.5f));
        for (int i = 0; i < 20; ++i)
        {
            int idx = random.nextInt((int)wavePoints.size());
            float x = width * idx / static_cast<float>(wavePoints.size() - 1);
            float y = height * (1.0f - wavePoints[idx]);
            float size = 1.0f + 2.0f * wavePoints[idx];
            
            g.fillEllipse(x - size/2, y - size/2, size, size);
        }
    }
    else if (animationMode == 1) // Bar mode
    {
        // Draw frequency spectrum as animated bars
        const int numBars = (int)spectrumValues.size();
        const float barWidth = width / static_cast<float>(numBars);
        
        for (int i = 0; i < numBars; ++i)
        {
            float barHeight = wavePoints[i] * height;
            
            // Create color gradient based on frequency and amplitude
            float intensity = 0.4f + 0.6f * wavePoints[i];
            float hue = colorScheme == 0 ? 0.6f - 0.2f * wavePoints[i] :  // Blue scheme
                        colorScheme == 1 ? 0.8f - 0.2f * wavePoints[i] :  // Purple scheme
                                          0.3f - 0.2f * wavePoints[i];    // Green scheme
            
            juce::Colour barColor = juce::Colour::fromHSV(hue, 0.7f, intensity, 1.0f);
            
            g.setColour(barColor);
            g.fillRect(i * barWidth, height - barHeight, barWidth, barHeight);
            
            // Add highlight effect at the top of the bar
            g.setColour(barColor.brighter(0.5f));
            g.fillRect(i * barWidth, height - barHeight, barWidth, 2.0f);
        }
    }
    else if (animationMode == 2) // Particle mode
    {
        // Draw frequency spectrum as animated particles
        const int numBars = (int)spectrumValues.size();
        const float barWidth = width / static_cast<float>(numBars);
        
        for (int i = 0; i < numBars; ++i)
        {
            float level = wavePoints[i];
            
            // Only draw particles for frequencies with some energy
            if (level > 0.05f)
            {
                // Number of particles depends on frequency energy
                int numParticles = static_cast<int>(level * 10.0f);
                
                for (int j = 0; j < numParticles; ++j)
                {
                    float particleSize = (1.0f + 4.0f * level) * (0.5f + 0.5f * random.nextFloat());
                    
                    // Particle position: x based on frequency, y based on amplitude with randomness
                    float x = i * barWidth + random.nextFloat() * barWidth;
                    float y = height - height * level * (0.5f + 0.5f * random.nextFloat());
                    
                    // Color based on frequency and scheme
                    float hue = colorScheme == 0 ? 0.6f - 0.2f * (static_cast<float>(i) / numBars) :  // Blue scheme
                                colorScheme == 1 ? 0.8f - 0.3f * (static_cast<float>(i) / numBars) :  // Purple scheme
                                                  0.3f - 0.2f * (static_cast<float>(i) / numBars);    // Green scheme
                    
                    juce::Colour particleColor = juce::Colour::fromHSV(hue, 0.8f, 0.9f, 0.7f);
                    
                    g.setColour(particleColor);
                    g.fillEllipse(x - particleSize/2, y - particleSize/2, particleSize, particleSize);
                }
            }
        }
    }
    
    // Draw frequency analyzer frame
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRect(getLocalBounds(), 1);
}

void SpectrumAnalyzerComponent::resized()
{
    // Nothing to do here as sizing is handled by the parent component
}

void SpectrumAnalyzerComponent::updateSpectrum(const float* spectrumData, int numBins)
{
    // Update spectrum values
    jassert(numBins == targetSpectrumValues.size());
    
    for (int i = 0; i < numBins; ++i)
    {
        targetSpectrumValues[i] = spectrumData[i];
    }
}

void SpectrumAnalyzerComponent::timerCallback()
{
    // Smooth spectrum values for display
    for (int i = 0; i < spectrumValues.size(); ++i)
    {
        previousSpectrumValues[i] = spectrumValues[i];
        spectrumValues[i] = previousSpectrumValues[i] + 
                          smoothingCoefficient * (targetSpectrumValues[i] - previousSpectrumValues[i]);
    }
    
    // Update wave animation
    updateAnimation();
    
    // Request a repaint
    repaint();
}

void SpectrumAnalyzerComponent::updateAnimation()
{
    // Update wave targets based on spectrum values
    for (int i = 0; i < waveTargets.size(); ++i)
    {
        waveTargets[i] = spectrumValues[i];
    }
    
    // Physics-based animation for wave points
    for (int i = 0; i < wavePoints.size(); ++i)
    {
        // Apply spring force towards target
        float force = tension * (waveTargets[i] - wavePoints[i]);
        waveVelocities[i] += force;
        
        // Apply velocity to position
        wavePoints[i] += waveVelocities[i] * animationSpeed;
        
        // Apply damping
        waveVelocities[i] *= damping;
    }
    
    // Apply spreading/smoothing between adjacent points (fluid-like behavior)
    std::vector<float> spreadForces(wavePoints.size(), 0.0f);
    
    for (int i = 1; i < wavePoints.size() - 1; ++i)
    {
        float leftDiff = wavePoints[i-1] - wavePoints[i];
        float rightDiff = wavePoints[i+1] - wavePoints[i];
        spreadForces[i] = spreadFactor * (leftDiff + rightDiff);
    }
    
    // Apply spread forces
    for (int i = 1; i < wavePoints.size() - 1; ++i)
    {
        waveVelocities[i] += spreadForces[i];
    }
}

void SpectrumAnalyzerComponent::setAnimationMode(int mode)
{
    animationMode = mode % 3; // Ensure it's 0, 1, or 2
}

void SpectrumAnalyzerComponent::setColorScheme(int scheme)
{
    colorScheme = scheme % 3; // Ensure it's 0, 1, or 2
    
    // Update colors based on scheme
    switch (colorScheme)
    {
        case 0: // Blue/Cyan
            baseColour1 = juce::Colours::blue;
            baseColour2 = juce::Colours::cyan;
            break;
        
        case 1: // Purple/Pink
            baseColour1 = juce::Colours::purple;
            baseColour2 = juce::Colours::pink;
            break;
        
        case 2: // Green/Yellow
            baseColour1 = juce::Colours::green;
            baseColour2 = juce::Colours::yellow;
            break;
    }
}

//==============================================================================
// CustomReverbAudioProcessorEditor Implementation
//==============================================================================

CustomReverbAudioProcessorEditor::CustomReverbAudioProcessorEditor (CustomReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      spectrumAnalyzer(p)
{
    // Set up custom look and feel
    customLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colour(100, 180, 240));
    customLookAndFeel.setColour(juce::Slider::trackColourId, juce::Colour(80, 90, 100));
    customLookAndFeel.setColour(juce::Slider::backgroundColourId, juce::Colour(40, 50, 60));
    
    // Set up each slider and its label
    auto& apvts = audioProcessor.getAPVTS();
    
    // Room Size Slider
    roomSizeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    roomSizeSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(roomSizeSlider);
    
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    roomSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(roomSizeLabel);
    
    roomSizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "roomSize", roomSizeSlider));
    
    // Damping Slider
    dampingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    dampingSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(dampingSlider);
    
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dampingLabel);
    
    dampingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "damping", dampingSlider));
    
    // Wet Level Slider
    wetLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    wetLevelSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(wetLevelSlider);
    
    wetLevelLabel.setText("Wet Level", juce::dontSendNotification);
    wetLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wetLevelLabel);
    
    wetLevelAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "wetLevel", wetLevelSlider));
    
    // Dry Level Slider
    dryLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    dryLevelSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(dryLevelSlider);
    
    dryLevelLabel.setText("Dry Level", juce::dontSendNotification);
    dryLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryLevelLabel);
    
    dryLevelAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "dryLevel", dryLevelSlider));
    
    // Width Slider
    widthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    widthSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(widthSlider);
    
    widthLabel.setText("Width", juce::dontSendNotification);
    widthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(widthLabel);
    
    widthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "width", widthSlider));
    
    // High Frequency Delay Slider
    highFreqDelaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    highFreqDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    highFreqDelaySlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(highFreqDelaySlider);
    
    highFreqDelayLabel.setText("High Freq Delay", juce::dontSendNotification);
    highFreqDelayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(highFreqDelayLabel);
    
    highFreqDelayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "highFreqDelay", highFreqDelaySlider));
    
    // Crossover Slider
    crossoverSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    crossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    crossoverSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(crossoverSlider);
    
    crossoverLabel.setText("Crossover", juce::dontSendNotification);
    crossoverLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crossoverLabel);
    
    crossoverAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "crossover", crossoverSlider));
    crossoverSlider.setRange(0.0f, 1.0f, 0.01f);
    crossoverSlider.setValue(0.5f);

    // Harmonic Detune Amount Slider
    harmDetuneAmountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    harmDetuneAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    harmDetuneAmountSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(harmDetuneAmountSlider);
    
    harmDetuneAmountLabel.setText("Harmonic Detune", juce::dontSendNotification);
    harmDetuneAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(harmDetuneAmountLabel);
    
    harmDetuneAmountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        apvts, "harmDetuneAmount", harmDetuneAmountSlider));  
    // Freeze Mode Button
    freezeModeButton.setButtonText("Freeze");
    addAndMakeVisible(freezeModeButton);
    
    freezeModeAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        apvts, "freezeMode", freezeModeButton));
    
    // Preset Selector
    setupPresetMenu();
    addAndMakeVisible(presetSelector);
    
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetLabel);
    
    // Spectrum analyzer
    addAndMakeVisible(spectrumAnalyzer);
    
    spectrumLabel.setText("Spectrum Analyzer", juce::dontSendNotification);
    spectrumLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(spectrumLabel);
    
    // Animation style and color buttons
    animationStyleButton.setButtonText("Animation: Wave");
    animationStyleButton.onClick = [this] { cycleAnimationStyle(); };
    addAndMakeVisible(animationStyleButton);
    
    colorSchemeButton.setButtonText("Color: Blue");
    colorSchemeButton.onClick = [this] { cycleColorScheme(); };
    addAndMakeVisible(colorSchemeButton);
    
    // Set the initial size of the editor
    setSize (600, 500);
}

CustomReverbAudioProcessorEditor::~CustomReverbAudioProcessorEditor()
{
    // Clean up the look and feel to avoid memory leaks
    roomSizeSlider.setLookAndFeel(nullptr);
    dampingSlider.setLookAndFeel(nullptr);
    wetLevelSlider.setLookAndFeel(nullptr);
    dryLevelSlider.setLookAndFeel(nullptr);
    widthSlider.setLookAndFeel(nullptr);
    highFreqDelaySlider.setLookAndFeel(nullptr);
    crossoverSlider.setLookAndFeel(nullptr);
    harmDetuneAmountSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void CustomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colour(30, 40, 50));
    
    // Draw a gradient background
    juce::ColourGradient gradient(
        juce::Colour(15, 25, 35), 0, 0,
        juce::Colour(40, 50, 60), (float)getWidth(), (float)getHeight(),
        false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
    
    // Draw title
    FontOptions fOptions(24.0f, juce::Font::bold);
    g.setFont(juce::Font(fOptions));
    g.setColour(juce::Colours::white);
    g.drawText("Custom Reverb Plugin", 20, 15, getWidth() - 40, 30, juce::Justification::centred);
}

void CustomReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto topArea = area.removeFromTop(40); // Title area
    
    // Spectrum analyzer takes up top portion
    auto spectrumArea = area.removeFromTop(200);
    spectrumLabel.setBounds(spectrumArea.removeFromTop(20));
    
    // Animation control buttons
    auto buttonArea = spectrumArea.removeFromBottom(30);
    animationStyleButton.setBounds(buttonArea.removeFromLeft(150));
    colorSchemeButton.setBounds(buttonArea.removeFromLeft(150));
    
    spectrumAnalyzer.setBounds(spectrumArea);
    
    // Parameter controls in the bottom section
    auto controlsArea = area.reduced(0, 10);
    
    // First row of controls
    auto row1 = controlsArea.removeFromTop(120);
    int sliderWidth = row1.getWidth() / 4;
    
    roomSizeSlider.setBounds(row1.removeFromLeft(sliderWidth).reduced(10));
    roomSizeLabel.setBounds(roomSizeSlider.getX(), roomSizeSlider.getY() - 15, 
                           roomSizeSlider.getWidth(), 20);
    
    dampingSlider.setBounds(row1.removeFromLeft(sliderWidth).reduced(10));
    dampingLabel.setBounds(dampingSlider.getX(), dampingSlider.getY() - 15, 
                          dampingSlider.getWidth(), 20);
    
    wetLevelSlider.setBounds(row1.removeFromLeft(sliderWidth).reduced(10));
    wetLevelLabel.setBounds(wetLevelSlider.getX(), wetLevelSlider.getY() - 15, 
                           wetLevelSlider.getWidth(), 20);
    
    dryLevelSlider.setBounds(row1.removeFromLeft(sliderWidth).reduced(10));
    dryLevelLabel.setBounds(dryLevelSlider.getX(), dryLevelSlider.getY() - 15, 
                           dryLevelSlider.getWidth(), 20);
    
    // Second row of controls
    auto row2 = controlsArea.removeFromTop(120);
    sliderWidth = row2.getWidth() / 5;
    
    widthSlider.setBounds(row2.removeFromLeft(sliderWidth).reduced(10));
    widthLabel.setBounds(widthSlider.getX(), widthSlider.getY() - 15, 
                        widthSlider.getWidth(), 20);
    
    highFreqDelaySlider.setBounds(row2.removeFromLeft(sliderWidth).reduced(10));
    highFreqDelayLabel.setBounds(highFreqDelaySlider.getX(), highFreqDelaySlider.getY() - 15, 
                                highFreqDelaySlider.getWidth(), 20);
    
    crossoverSlider.setBounds(row2.removeFromLeft(sliderWidth).reduced(10));
    crossoverLabel.setBounds(crossoverSlider.getX(), crossoverSlider.getY() - 15, crossoverSlider.getWidth(), 20);
    
    harmDetuneAmountSlider.setBounds(row2.removeFromLeft(sliderWidth).reduced(10));
    harmDetuneAmountLabel.setBounds(harmDetuneAmountSlider.getX(), harmDetuneAmountSlider.getY() - 15, 
                            harmDetuneAmountSlider.getWidth(), 20);
    
    // Bottom row with freeze mode and preset selector
    auto bottomRow = controlsArea.removeFromTop(40);
    freezeModeButton.setBounds(bottomRow.removeFromLeft(100).reduced(10));
    
    auto presetArea = bottomRow.reduced(10);
    presetLabel.setBounds(presetArea.removeFromLeft(60));
    presetSelector.setBounds(presetArea);
}

void CustomReverbAudioProcessorEditor::setupPresetMenu()
{
    presetSelector.addItem("Small Room", 1);
    presetSelector.addItem("Medium Room", 2);
    presetSelector.addItem("Large Hall", 3);
    presetSelector.addItem("Cathedral", 4);
    presetSelector.addItem("Special FX", 5);
    presetSelector.addItem("Bright Chamber", 6);
    presetSelector.addItem("Dark Space", 7);
    presetSelector.addItem("Harmonic Detuner", 8);
    
    presetSelector.onChange = [this] { loadPreset(presetSelector.getSelectedItemIndex()); };
    presetSelector.setSelectedItemIndex(0);
}

void CustomReverbAudioProcessorEditor::loadPreset(int presetIndex)
{
    auto& apvts = audioProcessor.getAPVTS();
    
    switch (presetIndex)
    {
        case 0: // Small Room
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.3f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.6f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.25f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.8f);
            apvts.getParameter("width")->setValueNotifyingHost(0.5f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.2f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.4f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 1: // Medium Room
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.5f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.5f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.33f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.7f);
            apvts.getParameter("width")->setValueNotifyingHost(0.7f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.3f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.5f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 2: // Large Hall
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.85f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.3f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.4f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.6f);
            apvts.getParameter("width")->setValueNotifyingHost(1.0f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.4f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.3f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 3: // Cathedral
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.95f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.2f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.5f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.5f);
            apvts.getParameter("width")->setValueNotifyingHost(1.0f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.7f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.2f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 4: // Special FX
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.9f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.1f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.9f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.2f);
            apvts.getParameter("width")->setValueNotifyingHost(1.0f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(1.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.8f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.7f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 5: // Bright Chamber
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.4f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.3f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.3f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.7f);
            apvts.getParameter("width")->setValueNotifyingHost(0.8f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.1f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.8f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
            
        case 6: // Dark Space
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.8f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.8f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.4f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.6f);
            apvts.getParameter("width")->setValueNotifyingHost(0.9f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.5f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.3f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.0f);
            break;
        case 7: // Harmonic Detuner
            apvts.getParameter("roomSize")->setValueNotifyingHost(0.4f);
            apvts.getParameter("damping")->setValueNotifyingHost(0.4f);
            apvts.getParameter("wetLevel")->setValueNotifyingHost(0.3f);
            apvts.getParameter("dryLevel")->setValueNotifyingHost(0.7f);
            apvts.getParameter("width")->setValueNotifyingHost(0.7f);
            apvts.getParameter("freezeMode")->setValueNotifyingHost(0.0f);
            apvts.getParameter("highFreqDelay")->setValueNotifyingHost(0.3f);
            apvts.getParameter("crossover")->setValueNotifyingHost(0.6f);
            apvts.getParameter("harmDetuneAmount")->setValueNotifyingHost(0.7f);
            break;
    }
}

void CustomReverbAudioProcessorEditor::cycleAnimationStyle()
{
    currentAnimationStyle = (currentAnimationStyle + 1) % 3;
    spectrumAnalyzer.setAnimationMode(currentAnimationStyle);
    
    // Update button text
    juce::String styleName;
    switch (currentAnimationStyle)
    {
        case 0:  styleName = "Wave"; break;
        case 1:  styleName = "Bars"; break;
        case 2:  styleName = "Particles"; break;
    }
    animationStyleButton.setButtonText("Animation: " + styleName);
}

void CustomReverbAudioProcessorEditor::cycleColorScheme()
{
    currentColorScheme = (currentColorScheme + 1) % 3;
    spectrumAnalyzer.setColorScheme(currentColorScheme);
    
    // Update button text
    juce::String schemeName;
    switch (currentColorScheme)
    {
        case 0:  schemeName = "Blue"; break;
        case 1:  schemeName = "Purple"; break;
        case 2:  schemeName = "Green"; break;
    }
    colorSchemeButton.setButtonText("Color: " + schemeName);
}
