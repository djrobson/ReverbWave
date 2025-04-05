/*
  ==============================================================================

    CustomReverbProcessor.cpp
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
  
  This file contains the implementation of the CustomReverbAudioProcessor class,
  which provides all the audio processing functionality for the VST plugin.
  
  Key features:
  - Realistic room reverberation using JUCE's Reverb class
  - Enhanced stereo field using harmonic detuning (odd/even harmonics)
  - Separate high-frequency delay for natural sound decay
  - Spectrum analysis for visualization
  - Parameter management through JUCE's AudioProcessorValueTreeState
  
  The processor handles multiple reverb parameters including:
  - Room size, damping, and decay time
  - Wet/dry mix controls
  - Stereo width enhancement
  - Freeze mode for infinite sustain
  - High-frequency delay for more natural decay
  - Crossover frequency for multi-band processing
  - Harmonic detuning amount for enhanced stereo imaging
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CustomReverbAudioProcessor::CustomReverbAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann),
      apvts(*this, nullptr, "Parameters", createParameters())
{
    // Initialize memory for high frequency delay
    highFreqBufferSize = 44100; // 1 second at 44.1kHz
    highFreqDelayBufferL.resize(highFreqBufferSize, 0.0f);
    highFreqDelayBufferR.resize(highFreqBufferSize, 0.0f);
    
    // Initialize harmonic detuning buffers
    oddHarmonicBufferL.resize(maxHarmonicFilterSize, 0.0f);
    evenHarmonicBufferR.resize(maxHarmonicFilterSize, 0.0f);
    
    // Set default reverb parameters
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.33f;
    reverbParams.dryLevel = 0.4f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    
    // Apply to reverb processor
    reverb.setParameters(reverbParams);
    
    // Add parameter listeners
    auto parameterNames = {"roomSize", "damping", "wetLevel", "dryLevel", 
                           "width", "freezeMode", "highFreqDelay", "crossover", "harmDetuneAmount"};
    
    for (auto& name : parameterNames)
        apvts.addParameterListener(name, this);
}

CustomReverbAudioProcessor::~CustomReverbAudioProcessor()
{
    // Remove parameter listeners to avoid callbacks after destruction
    auto parameterNames = {"roomSize", "damping", "wetLevel", "dryLevel", 
                           "width", "freezeMode", "highFreqDelay", "crossover", "harmDetuneAmount"};
    
    for (auto& name : parameterNames)
        apvts.removeParameterListener(name, this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CustomReverbAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Standard reverb parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("damping", "Damping", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("wetLevel", "Wet Level", 0.0f, 1.0f, 0.33f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("dryLevel", "Dry Level", 0.0f, 1.0f, 0.4f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("width", "Width", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("freezeMode", "Freeze Mode", 0.0f, 1.0f, 0.0f));
    
    // Custom parameters for high frequency processing
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("highFreqDelay", "High Freq Delay", 0.0f, 1.0f, 0.3f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("crossover", "Crossover", 0.0f, 1.0f, 0.5f));
    
    // Harmonic detuning parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("harmDetuneAmount", "Harmonic Detune", 0.0f, 1.0f, 0.0f));
    
    return { parameters.begin(), parameters.end() };
}

void CustomReverbAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    // Update parameter in the custom params struct
    if (parameterID == "roomSize")
        customParams.roomSize = newValue;
    else if (parameterID == "damping")
        customParams.damping = newValue;
    else if (parameterID == "wetLevel")
        customParams.wetLevel = newValue;
    else if (parameterID == "dryLevel")
        customParams.dryLevel = newValue;
    else if (parameterID == "width")
        customParams.width = newValue;
    else if (parameterID == "freezeMode")
        customParams.freezeMode = newValue;
    else if (parameterID == "highFreqDelay")
        customParams.highFreqDelay = newValue;
    else if (parameterID == "crossover")
        customParams.crossover = newValue;
    else if (parameterID == "harmDetuneAmount")
        customParams.harmDetuneAmount = newValue;
    
    // Update the reverb parameters
    updateReverbParameters();
    updateHighFreqParameters();
}

void CustomReverbAudioProcessor::updateReverbParameters()
{
    // Transfer from custom params to JUCE reverb params
    reverbParams.roomSize = customParams.roomSize;
    reverbParams.damping = customParams.damping;
    reverbParams.wetLevel = customParams.wetLevel;
    reverbParams.dryLevel = customParams.dryLevel;
    reverbParams.width = customParams.width;
    reverbParams.freezeMode = customParams.freezeMode;
    
    // Apply to the reverb processor
    reverb.setParameters(reverbParams);
}

void CustomReverbAudioProcessor::updateHighFreqParameters()
{
    // Calculate high frequency delay amount from parameter
    highFreqDelayAmount = customParams.highFreqDelay * 0.02f; // Max 20ms
    
    // Calculate crossover frequency coefficient (from 500Hz to 8kHz)
    float crossoverFreq = 500.0f * std::pow(16.0f, customParams.crossover);
    float rc = 1.0f / (2.0f * juce::MathConstants<float>::pi * crossoverFreq);
    float dt = 1.0f / getSampleRate();
    lowpassCoeff = dt / (rc + dt);
}

void CustomReverbAudioProcessor::splitFrequencies(float input, float& lowOut, float& highOut, float& state)
{
    // Simple first-order lowpass filter to split frequencies
    lowOut = state + lowpassCoeff * (input - state);
    state = lowOut;
    highOut = input - lowOut;
}

float CustomReverbAudioProcessor::processHighFreqDelay(float input, std::vector<float>& buffer)
{
    // Store the sample in the buffer
    buffer[highFreqDelayWritePos] = input;
    
    // Calculate delay sample position
    int delayLength = static_cast<int>(highFreqDelayAmount * getSampleRate());
    int readPos = highFreqDelayWritePos - delayLength;
    if (readPos < 0)
        readPos += highFreqBufferSize;
    
    // Get delayed sample
    float delayedSample = buffer[readPos];
    
    // Update write position
    highFreqDelayWritePos = (highFreqDelayWritePos + 1) % highFreqBufferSize;
    
    return delayedSample;
}

void CustomReverbAudioProcessor::processHarmonicDetuning(float& leftSample, float& rightSample)
{
    // Skip processing if detune amount is zero
    if (customParams.harmDetuneAmount <= 0.001f)
        return;
    
    // Get the detune amount (0-1 maps to 0-10 Hz shift)
    float detuneAmount = customParams.harmDetuneAmount * 10.0f;
    
    // Store samples in odd/even harmonic buffers
    oddHarmonicBufferL[oddHarmonicPos] = leftSample;
    evenHarmonicBufferR[evenHarmonicPos] = rightSample;
    
    // Calculate the phase shift amount for the sample rate
    float phaseShiftSamples = detuneAmount / getSampleRate() * maxHarmonicFilterSize;
    
    // Detune odd harmonics in left channel
    int readPos = oddHarmonicPos - static_cast<int>(phaseShiftSamples) % maxHarmonicFilterSize;
    if (readPos < 0)
        readPos += maxHarmonicFilterSize;
    leftSample = oddHarmonicBufferL[readPos];
    
    // Detune even harmonics in right channel (opposite direction)
    readPos = evenHarmonicPos + static_cast<int>(phaseShiftSamples) % maxHarmonicFilterSize;
    if (readPos >= maxHarmonicFilterSize)
        readPos -= maxHarmonicFilterSize;
    rightSample = evenHarmonicBufferR[readPos];
    
    // Update buffer positions
    oddHarmonicPos = (oddHarmonicPos + 1) % maxHarmonicFilterSize;
    evenHarmonicPos = (evenHarmonicPos + 1) % maxHarmonicFilterSize;
}

//==============================================================================
const juce::String CustomReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CustomReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CustomReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CustomReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CustomReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CustomReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CustomReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CustomReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CustomReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void CustomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CustomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the reverb for the sample rate
    reverb.reset();
    
    // Reset FFT stuff
    fifoIndex = 0;
    nextFFTBlockReady = false;
    
    // Clear buffers and states
    std::fill(fifo, fifo + fftSize, 0.0f);
    std::fill(fftData, fftData + 2 * fftSize, 0.0f);
    std::fill(scopeData, scopeData + scopeSize, 0.0f);
    
    // Reset high frequency delay buffers
    std::fill(highFreqDelayBufferL.begin(), highFreqDelayBufferL.end(), 0.0f);
    std::fill(highFreqDelayBufferR.begin(), highFreqDelayBufferR.end(), 0.0f);
    highFreqDelayReadPos = 0;
    highFreqDelayWritePos = 0;
    
    // Reset harmonic detuning buffers
    std::fill(oddHarmonicBufferL.begin(), oddHarmonicBufferL.end(), 0.0f);
    std::fill(evenHarmonicBufferR.begin(), evenHarmonicBufferR.end(), 0.0f);
    oddHarmonicPos = 0;
    evenHarmonicPos = 0;
    
    // Reset filter states
    lowpassStateL = 0.0f;
    lowpassStateR = 0.0f;
    
    // Update parameters for the sample rate
    updateHighFreqParameters();
}

void CustomReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CustomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Supports mono or stereo input/output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
        
    // Only support matching input/output channel count
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void CustomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get audio data pointers
    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : channelDataL;
    
    // Process each sample
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float inputL = channelDataL[i];
        float inputR = channelDataR[i];
        
        // Split into low and high frequency components
        float lowL, highL, lowR, highR;
        splitFrequencies(inputL, lowL, highL, lowpassStateL);
        splitFrequencies(inputR, lowR, highR, lowpassStateR);
        
        // Process high frequencies with delay
        float delayedHighL = processHighFreqDelay(highL, highFreqDelayBufferL);
        float delayedHighR = processHighFreqDelay(highR, highFreqDelayBufferR);
        
        // Apply harmonic detuning (before reverb for subtle effect)
        float mixedL = lowL + delayedHighL;
        float mixedR = lowR + delayedHighR;
        processHarmonicDetuning(mixedL, mixedR);
        
        // Process the mixed signal with the reverb
        float reverbInputs[2] = { mixedL, mixedR };
        float reverbOutputs[2] = { 0.0f, 0.0f };
        reverb.processStereo(reverbInputs, reverbOutputs, 1);
        
        // Store the processed audio back to the buffer
        channelDataL[i] = reverbOutputs[0];
        if (totalNumOutputChannels > 1)
            channelDataR[i] = reverbOutputs[1];
        
        // Feed spectral analyzer (mono mix)
        float monoSample = 0.5f * (reverbOutputs[0] + (totalNumOutputChannels > 1 ? reverbOutputs[1] : reverbOutputs[0]));
        pushNextSampleIntoFifo(monoSample);
    }
    
    // Update spectrum analyzer if one is attached
    if (nextFFTBlockReady && spectrumAnalyzer != nullptr)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
    }
}

void CustomReverbAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    // If the fifo contains enough data, set a flag to say the next fft should be calculated
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            std::fill(fftData, fftData + 2 * fftSize, 0.0f);
            std::copy(fifo, fifo + fftSize, fftData);
            nextFFTBlockReady = true;
        }
        
        fifoIndex = 0;
    }
    
    // Add sample to the fifo
    fifo[fifoIndex++] = sample;
}

void CustomReverbAudioProcessor::drawNextFrameOfSpectrum()
{
    // Apply windowing function to the data
    window.multiplyWithWindowingTable(fftData, fftSize);
    
    // Perform the FFT
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);
    
    // Calculate the spectrum data for display
    float minDb = -100.0f;
    float maxDb = 0.0f;
    
    for (int i = 0; i < scopeSize; ++i)
    {
        // Select frequencies logarithmically for better display
        auto index = static_cast<int>(1.0f * i / scopeSize * (fftSize / 2));
        
        // Find the magnitude
        auto level = fftData[index];
        
        // Convert to decibels with normalization and limiting
        level = juce::jmax(minDb, juce::Decibels::gainToDecibels(level) - juce::Decibels::gainToDecibels((float)fftSize));
        level = juce::jmap(level, minDb, maxDb, 0.0f, 1.0f);
        
        // Map to scopeData
        scopeData[i] = level;
    }
    
    // Send the data to the spectrum analyzer if available
    if (spectrumAnalyzer != nullptr)
        spectrumAnalyzer->updateSpectrum(scopeData, scopeSize);
}

void CustomReverbAudioProcessor::setSpectrumAnalyzer(SpectrumAnalyzerComponent* analyzer)
{
    spectrumAnalyzer = analyzer;
}

//==============================================================================
bool CustomReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CustomReverbAudioProcessor::createEditor()
{
    return new CustomReverbAudioProcessorEditor (*this);
}

//==============================================================================
void CustomReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create XML for saving plugin state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CustomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore plugin state from XML
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        updateReverbParameters();
        updateHighFreqParameters();
    }
}

//==============================================================================
// This creates new instances of the plugin
// Make sure this is explicitly defined for VST3 compatibility
// Required for VST3 plugins to properly initialize
extern "C" JUCE_EXPORT juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
