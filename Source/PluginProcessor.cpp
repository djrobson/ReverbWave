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

// Define M_PI for Windows if it's not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    // Set up default reverb parameters
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.33f;
    reverbParams.dryLevel = 0.4f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    
    // Initialize crossover frequency
    customParams.crossover = 2000.0f;
    customParams.highFreqDelay = 0.1f; // 100ms default
    customParams.highFreqDelayMix = 0.3f;
    
    // Initialize harmonic detuning
    customParams.harmDetuneAmount = 0.5f;
    
    // Setup spectrum analyzer
    //fftSize = 1 << fftOrder;
    //fifo.resize(fftSize, 0.0f);
    //fftData = new float[2 * fftSize];
    //scopeData = new float[scopeSize];
    std::fill(fftData, fftData + 2 * fftSize, 0.0f);
    std::fill(scopeData, scopeData + scopeSize, 0.0f);
    fifoIndex = 0;
    nextFFTBlockReady = false;
    
    // Initialize the reverb processors
    leftReverb.reset();
    rightReverb.reset();
    leftReverb.setParameters(reverbParams);
    rightReverb.setParameters(reverbParams);
    
    // Initialize parameter listeners
    apvts.addParameterListener("roomSize", this);
    apvts.addParameterListener("damping", this);
    apvts.addParameterListener("wetLevel", this);
    apvts.addParameterListener("dryLevel", this);
    apvts.addParameterListener("width", this);
    apvts.addParameterListener("freezeMode", this);
    apvts.addParameterListener("crossoverFreq", this);
    apvts.addParameterListener("highFreqDelay", this);
    apvts.addParameterListener("highFreqMix", this);
    apvts.addParameterListener("harmonicDetune", this);
}

CustomReverbAudioProcessor::~CustomReverbAudioProcessor()
{
    // Clean up memory
    //delete[] fftData;
    //delete[] scopeData;
    
    // Remove parameter listeners
    apvts.removeParameterListener("roomSize", this);
    apvts.removeParameterListener("damping", this);
    apvts.removeParameterListener("wetLevel", this);
    apvts.removeParameterListener("dryLevel", this);
    apvts.removeParameterListener("width", this);
    apvts.removeParameterListener("freezeMode", this);
    apvts.removeParameterListener("crossoverFreq", this);
    apvts.removeParameterListener("highFreqDelay", this);
    apvts.removeParameterListener("highFreqMix", this);
    apvts.removeParameterListener("harmonicDetune", this);
}

//==============================================================================
void CustomReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Update parameters based on changes in the GUI or automation
    if (parameterID == "roomSize")
        reverbParams.roomSize = newValue;
    else if (parameterID == "damping")
        reverbParams.damping = newValue;
    else if (parameterID == "wetLevel")
        reverbParams.wetLevel = newValue;
    else if (parameterID == "dryLevel")
        reverbParams.dryLevel = newValue;
    else if (parameterID == "width")
        reverbParams.width = newValue;
    else if (parameterID == "freezeMode")
        reverbParams.freezeMode = newValue;
    else if (parameterID == "crossoverFreq")
        customParams.crossover = 20.0f * std::pow(1000.0f, newValue);
    else if (parameterID == "highFreqDelay")
        customParams.highFreqDelay = juce::jmap(newValue, 0.001f, 0.5f);
    else if (parameterID == "highFreqMix")
        customParams.highFreqDelayMix = newValue;
    else if (parameterID == "harmonicDetune")
        customParams.harmDetuneAmount = newValue;
    
    // Update the reverb processors with new parameters
    if (parameterID == "roomSize" || parameterID == "damping" || 
        parameterID == "wetLevel" || parameterID == "dryLevel" || 
        parameterID == "width" || parameterID == "freezeMode")
    {
        updateReverbParameters();
    }
}

void CustomReverbAudioProcessor::updateReverbParameters()
{
    leftReverb.setParameters(reverbParams);
    rightReverb.setParameters(reverbParams);
}

void CustomReverbAudioProcessor::updateHighFreqParameters()
{
    // Calculate delay time in samples based on the sample rate
    int highFreqDelaySamples = static_cast<int>(customParams.highFreqDelay * getSampleRate());
    
    // Ensure delay buffer is large enough
    if (highFreqDelaySamples > highFreqBufferSize)
    {
        highFreqBufferSize = highFreqDelaySamples;
        highFreqDelayBufferL.resize(highFreqBufferSize, 0.0f);
        highFreqDelayBufferR.resize(highFreqBufferSize, 0.0f);
    }
    
    // Reset delay buffer write position when parameters change significantly
    highFreqDelayWritePos = 0;
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
    (void)index; // Suppress unused parameter warning
}

const juce::String CustomReverbAudioProcessor::getProgramName (int index)
{
    (void)index; // Suppress unused parameter warning
    return {};
}

void CustomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    (void)index; // Suppress unused parameter warning
    (void)newName; // Suppress unused parameter warning
}

//==============================================================================
void CustomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    (void)samplesPerBlock; // Suppress unused parameter warning
    
    customParams.sampleRate = sampleRate;
    // Update parameters that depend on sample rate
    updateHighFreqParameters();
    
    // Prepare reverb processors
    leftReverb.reset();
    rightReverb.reset();
    //TODO: match samplesPerBlock to array sizes
    // Clear FFT and spectrum data
    std::fill(fftData, fftData + 2 * fftSize, 0.0f);
    std::fill(scopeData, scopeData + scopeSize, 0.0f);
    
    // Reset FIFO
    fifoIndex = 0;
    nextFFTBlockReady = false;
}

void CustomReverbAudioProcessor::releaseResources()
{
    // When playback stops, release any allocated resources
}

bool CustomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is a stereo processor - we only support stereo ins and outs
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}
// Process harmonic detuning on stereo channels
void CustomReverbAudioProcessor::processHarmonicDetuning(float& leftSample, float& rightSample) {
    if (customParams.harmDetuneAmount <= 0.001f) {
        return; // Skip processing if detuning is disabled
    }
    
    // Get the detune amount (0-1 maps to 0-10 Hz shift)
    float detuneAmount = customParams.harmDetuneAmount * 10.0f;
    
    // Store samples in odd/even harmonic buffers
    oddHarmonicBufferL[oddHarmonicPos] = leftSample;
    evenHarmonicBufferR[evenHarmonicPos] = rightSample;
    
    // Calculate the phase shift amount for the sample rate
    float phaseShiftSamples = detuneAmount / customParams.sampleRate * maxHarmonicFilterSize;
    
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

void CustomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    (void)midiMessages; // Suppress unused parameter warning
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Get pointers to the left and right channels
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // Process the audio - sample by sample
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float leftSample = leftChannel[sample];
        float rightSample = rightChannel[sample];
        
        // Calculate mono mix for spectrum analyzer
        float monoSample = (leftSample + rightSample) * 0.5f;
        pushNextSampleIntoFifo(monoSample);
        
        // Split the signal into low and high frequency bands
        float leftLow, leftHigh, rightLow, rightHigh;
        processCrossover(leftSample, rightSample, leftLow, leftHigh, rightLow, rightHigh);
        
        // Process low frequencies through the main reverb
        //TODO: Add a stereo reverb for left and right channels
        float leftLowReverb = 0; //leftReverb.processSample(leftLow);
        float rightLowReverb = 0; //rightReverb.processSample(rightLow);
        
        // Process high frequencies through the delay
        float leftHighDelay, rightHighDelay;
        processHighFreqDelay(leftHigh, rightHigh, leftHighDelay, rightHighDelay);
        
        // Apply harmonic detuning for stereo enhancement
        float leftOut = leftLowReverb + leftHighDelay;
        float rightOut = rightLowReverb + rightHighDelay;
        
        // Apply harmonic detuning if enabled
        if (customParams.harmDetuneAmount > 0.001f)
        {
            processHarmonicDetuning(leftOut, rightOut);
        }
        
        // Output the processed samples
        leftChannel[sample] = leftOut;
        rightChannel[sample] = rightOut;
    }
    
    // Update FFT display if it's time
    if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
    }
}

void CustomReverbAudioProcessor::processCrossover(float leftIn, float rightIn, 
                                               float& leftLow, float& leftHigh, 
                                               float& rightLow, float& rightHigh)
{
    // Simple one-pole low-pass/high-pass filter for crossover
    static float leftLP = 0.0f, rightLP = 0.0f;
    
    // Calculate filter coefficient from crossover frequency
    float sampleRate = (float)getSampleRate();
    float alpha = 1.0f - (float)std::exp(-2.0f * M_PI * customParams.crossover / sampleRate);
    
    // Process crossover
    leftLP = leftLP + alpha * (leftIn - leftLP);
    rightLP = rightLP + alpha * (rightIn - rightLP);
    
    leftLow = leftLP;
    rightLow = rightLP;
    
    leftHigh = leftIn - leftLow;
    rightHigh = rightIn - rightLow;
}

void CustomReverbAudioProcessor::processHighFreqDelay(float leftIn, float rightIn, 
                                                  float& leftOut, float& rightOut)
{
    // Calculate read position
    highFreqDelayReadPos = highFreqDelayWritePos - highFreqBufferSize;
    if (highFreqDelayReadPos < 0)
        highFreqDelayReadPos += highFreqBufferSize;
    
    // Get delayed samples
    float leftDelayed = highFreqDelayBufferL[highFreqDelayReadPos];
    float rightDelayed = highFreqDelayBufferR[highFreqDelayReadPos];
    
    // Write new samples to buffer
    highFreqDelayBufferL[highFreqDelayWritePos] = leftIn;
    highFreqDelayBufferR[highFreqDelayWritePos] = rightIn;
    
    // Update write position
    highFreqDelayWritePos++;
    if (highFreqDelayWritePos >= highFreqBufferSize)
        highFreqDelayWritePos = 0;
    
    // Mix original and delayed signals
    leftOut = leftIn * (1.0f - customParams.highFreqDelayMix) + leftDelayed * customParams.highFreqDelayMix;
    rightOut = rightIn * (1.0f - customParams.highFreqDelayMix) + rightDelayed * customParams.highFreqDelayMix;
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CustomReverbAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Reverb parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("damping", "Damping", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("wetLevel", "Wet Level", 0.0f, 1.0f, 0.33f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("dryLevel", "Dry Level", 0.0f, 1.0f, 0.4f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("width", "Width", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("freezeMode", "Freeze Mode", 0.0f, 1.0f, 0.0f));
    
    // Advanced parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("crossoverFreq", "Crossover Freq", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("highFreqDelay", "HF Delay", 0.0f, 1.0f, 0.2f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("highFreqMix", "HF Mix", 0.0f, 1.0f, 0.3f));
    
    // Harmonic detuning for stereo enhancement
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("harmonicDetune", "Harmonic Detune", 0.0f, 1.0f, 0.5f));
    
    return { parameters.begin(), parameters.end() };
}

void CustomReverbAudioProcessor::pushNextSampleIntoFifo(float sample)
{
    // If the fifo contains enough data, set a flag to say
    // that the next frame should be rendered
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

//juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
