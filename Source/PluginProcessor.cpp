/*
  ==============================================================================

    CustomReverbProcessor.cpp
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CustomReverbAudioProcessor::CustomReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    forwardFFT(fftOrder),
    window(fftSize, juce::dsp::WindowingFunction<float>::hann),
    apvts(*this, nullptr, "Parameters", createParameters())
{
    // Set up listeners for parameters
    auto parameterList = {"roomSize", "damping", "wetLevel", "dryLevel", "width", "freezeMode", "highFreqDelay", "crossover"};
    for (const auto& param : parameterList)
    {
        apvts.addParameterListener(param, this);
    }
    
    // Initialize reverb parameters
    updateReverbParameters();
    
    // Initialize FFT data
    memset(fifo, 0, sizeof(fifo));
    memset(fftData, 0, sizeof(fftData));
    memset(scopeData, 0, sizeof(scopeData));
}

CustomReverbAudioProcessor::~CustomReverbAudioProcessor()
{
    // Remove parameter listeners
    auto parameterList = {"roomSize", "damping", "wetLevel", "dryLevel", "width", "freezeMode", "highFreqDelay", "crossover"};
    for (const auto& param : parameterList)
    {
        apvts.removeParameterListener(param, this);
    }
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
    // Initialize reverb with the current sample rate
    reverb.setSampleRate(sampleRate);
    
    // Calculate buffer size for high frequency delay (max 200ms)
    highFreqBufferSize = (int)(sampleRate * 0.2);
    highFreqDelayBufferL.resize(highFreqBufferSize, 0.0f);
    highFreqDelayBufferR.resize(highFreqBufferSize, 0.0f);
    highFreqDelayReadPos = 0;
    highFreqDelayWritePos = 0;
    
    // Initialize lowpass filter coefficient (cutoff around 2kHz)
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
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
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
    
    // Get raw pointers to the input/output sample data
    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(totalNumInputChannels > 1 ? 1 : 0);
    
    // Process each sample
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        // Split signal into low and high frequencies
        float lowL, highL;
        splitFrequencies(channelDataL[i], lowL, highL, lowpassStateL);
        
        float lowR, highR;
        if (totalNumInputChannels > 1) 
        {
            splitFrequencies(channelDataR[i], lowR, highR, lowpassStateR);
        }
        else 
        {
            // If mono input, copy the left channel values
            lowR = lowL;
            highR = highL;
        }
        
        // Process high frequency delay
        float delayedHighL = processHighFreqDelay(highL, highFreqDelayBufferL);
        float delayedHighR = processHighFreqDelay(highR, highFreqDelayBufferR);
        
        // Combine low and delayed high frequencies
        channelDataL[i] = lowL + delayedHighL;
        if (totalNumInputChannels > 1) 
        {
            channelDataR[i] = lowR + delayedHighR;
        }
        else
        {
            channelDataR[i] = channelDataL[i];
        }

        // Input to FFT analyzer (mixing down to mono)
        pushNextSampleIntoFifo((channelDataL[i] + channelDataR[i]) * 0.5f);
    }
    
    // Apply standard reverb processing
    reverb.processStereo(channelDataL, channelDataR, buffer.getNumSamples());
    
    // Check if we need to update the spectrum
    if (nextFFTBlockReady && spectrumAnalyzer != nullptr)
    {
        calculateFrequencySpectrum();
        spectrumAnalyzer->updateSpectrum(scopeData, scopeSize);
        nextFFTBlockReady = false;
    }
}

void CustomReverbAudioProcessor::splitFrequencies(float input, float& lowOut, float& highOut, float& state)
{
    // Simple first-order lowpass filter
    // lowpass = a * input + (1 - a) * lowpass_prev
    lowOut = lowpassCoeff * input + (1.0f - lowpassCoeff) * state;
    state = lowOut;
    
    // Highpass is just the remainder
    highOut = input - lowOut;
}

float CustomReverbAudioProcessor::processHighFreqDelay(float input, std::vector<float>& buffer)
{
    // Apply delay to the high frequencies
    float output = buffer[highFreqDelayReadPos];
    
    // Write new input to the buffer
    buffer[highFreqDelayWritePos] = input;
    
    // Update read/write positions
    highFreqDelayReadPos = (highFreqDelayReadPos + 1) % highFreqBufferSize;
    highFreqDelayWritePos = (highFreqDelayWritePos + 1) % highFreqBufferSize;
    
    return output;
}

void CustomReverbAudioProcessor::updateHighFreqParameters()
{
    // Update crossover frequency (range from ~500Hz to ~3kHz)
    float crossoverFreq = 500.0f * std::pow(6.0f, customParams.crossover);
    
    // Simple RC lowpass coefficient calculation
    // where a = dt / (RC + dt) and RC = 1 / (2 * pi * cutoff_freq)
    float sampleRate = (float)getSampleRate();
    if (sampleRate > 0)
    {
        lowpassCoeff = 1.0f / (1.0f + 2.0f * juce::MathConstants<float>::pi * crossoverFreq / sampleRate);
    }
    
    // Update delay time (range from 0 to 20ms)
    if (sampleRate > 0)
    {
        int delaySamples = (int)(sampleRate * 0.02f * customParams.highFreqDelay);
        highFreqDelayAmount = delaySamples;
        
        // Adjust read position based on write position and delay amount
        highFreqDelayReadPos = (highFreqDelayWritePos - delaySamples + highFreqBufferSize) % highFreqBufferSize;
    }
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
    // Save state information
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CustomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore state information
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
            updateReverbParameters();
        }
    }
}

void CustomReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Update the custom parameter value
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
    {
        customParams.highFreqDelay = newValue;
        updateHighFreqParameters();
    }
    else if (parameterID == "crossover")
    {
        customParams.crossover = newValue;
        updateHighFreqParameters();
    }
    
    // Update the reverb parameters
    updateReverbParameters();
}

void CustomReverbAudioProcessor::updateReverbParameters()
{
    // Set standard reverb parameters
    reverbParams.roomSize = customParams.roomSize;
    reverbParams.damping = customParams.damping;
    reverbParams.wetLevel = customParams.wetLevel;
    reverbParams.dryLevel = customParams.dryLevel;
    reverbParams.width = customParams.width;
    reverbParams.freezeMode = customParams.freezeMode;
    
    // Update the reverb with new parameters
    reverb.setParameters(reverbParams);
}

juce::AudioProcessorValueTreeState::ParameterLayout CustomReverbAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    params.add(std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.0f, 1.0f, 0.5f));
    params.add(std::make_unique<juce::AudioParameterFloat>("damping", "Damping", 0.0f, 1.0f, 0.5f));
    params.add(std::make_unique<juce::AudioParameterFloat>("wetLevel", "Wet Level", 0.0f, 1.0f, 0.33f));
    params.add(std::make_unique<juce::AudioParameterFloat>("dryLevel", "Dry Level", 0.0f, 1.0f, 0.4f));
    params.add(std::make_unique<juce::AudioParameterFloat>("width", "Width", 0.0f, 1.0f, 1.0f));
    params.add(std::make_unique<juce::AudioParameterBool>("freezeMode", "Freeze Mode", false));
    params.add(std::make_unique<juce::AudioParameterFloat>("highFreqDelay", "High Freq Delay", 0.0f, 1.0f, 0.3f));
    params.add(std::make_unique<juce::AudioParameterFloat>("crossover", "Crossover", 0.0f, 1.0f, 0.5f));
    
    return params;
}

// Spectrum analyzer and FFT methods
void CustomReverbAudioProcessor::setSpectrumAnalyzer(SpectrumAnalyzerComponent* analyzer)
{
    spectrumAnalyzer = analyzer;
}

void CustomReverbAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    // If the fifo contains enough data, set a flag to say
    // that the next line should now be rendered
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            std::memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        
        fifoIndex = 0;
    }
    
    fifo[fifoIndex++] = sample;
}

void CustomReverbAudioProcessor::calculateFrequencySpectrum()
{
    // First apply a windowing function to the data
    window.multiplyWithWindowingTable(fftData, fftSize);
    
    // Then render the FFT data
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);
    
    // Find the range of values produced, so we can scale the result
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData, fftSize / 2);
    
    // Convert the FFT data into a logarithmic spectrum with a reasonable 
    // range for the visual display
    for (int i = 0; i < scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
        auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
        
        scopeData[i] = level;
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
