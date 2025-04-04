/*
  ==============================================================================

    CustomReverbProcessor.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Forward declaration for spectrum analyzer access
class SpectrumAnalyzerComponent;

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
    
    // Spectrum analyzer access
    void setSpectrumAnalyzer(SpectrumAnalyzerComponent* analyzer);
    
    // FFT-related methods
    enum
    {
        fftOrder = 11,               // 2048 samples for FFT
        fftSize  = 1 << fftOrder,    // Size based on the order
        scopeSize = 512              // Number of points to display
    };

private:
    //==============================================================================
    // Custom reverb parameters
    struct CustomReverbParameters {
        float roomSize = 0.5f;
        float damping = 0.5f;
        float wetLevel = 0.33f;
        float dryLevel = 0.4f;
        float width = 1.0f;
        float freezeMode = 0.0f;
        float highFreqDelay = 0.3f;
        float crossover = 0.5f;
    };
    
    // Reverb processor and parameters
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    CustomReverbParameters customParams;
    
    // Audio processor value tree state for parameter management
    juce::AudioProcessorValueTreeState apvts;
    
    // Parameter creation helper
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    // Update reverb parameters based on current values
    void updateReverbParameters();
    
    // High frequency delay buffer for stereo
    std::vector<float> highFreqDelayBufferL;
    std::vector<float> highFreqDelayBufferR;
    int highFreqBufferSize = 0;
    int highFreqDelayReadPos = 0;
    int highFreqDelayWritePos = 0;
    float highFreqDelayAmount = 0.0f;
    float lowpassCoeff = 0.0f;
    float lowpassStateL = 0.0f;
    float lowpassStateR = 0.0f;
    
    // Crossover filter to split frequencies
    void splitFrequencies(float input, float& lowOut, float& highOut, float& state);
    
    // Process high frequency delay
    float processHighFreqDelay(float input, std::vector<float>& buffer);
    
    // Update high frequency processing parameters
    void updateHighFreqParameters();
    
    // Spectrum analyzer
    SpectrumAnalyzerComponent* spectrumAnalyzer = nullptr;
    
    // FFT analysis objects and data
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];
    
    // FFT processing methods
    void pushNextSampleIntoFifo(float sample) noexcept;
    void drawNextFrameOfSpectrum();
    void calculateFrequencySpectrum();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessor)
};
