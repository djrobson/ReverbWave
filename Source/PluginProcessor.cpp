/*
  ==============================================================================

    PluginProcessor.cpp
    Created: Custom Reverb Plugin
    Author:  Audio Plugin Developer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Define the parameter IDs
const juce::String CustomReverbAudioProcessor::roomSizeID = "roomSize";
const juce::String CustomReverbAudioProcessor::dampingID = "damping";
const juce::String CustomReverbAudioProcessor::widthID = "width";
const juce::String CustomReverbAudioProcessor::wetLevelID = "wetLevel";
const juce::String CustomReverbAudioProcessor::dryLevelID = "dryLevel";

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
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Register as a listener to all parameters
    parameters.addParameterListener(roomSizeID, this);
    parameters.addParameterListener(dampingID, this);
    parameters.addParameterListener(widthID, this);
    parameters.addParameterListener(wetLevelID, this);
    parameters.addParameterListener(dryLevelID, this);
    
    // Initialize reverb parameters with default values
    updateReverbParameters();
}

CustomReverbAudioProcessor::~CustomReverbAudioProcessor()
{
    // Remove parameter listeners
    parameters.removeParameterListener(roomSizeID, this);
    parameters.removeParameterListener(dampingID, this);
    parameters.removeParameterListener(widthID, this);
    parameters.removeParameterListener(wetLevelID, this);
    parameters.removeParameterListener(dryLevelID, this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CustomReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Room size - controls the reverb time
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        roomSizeID, "Room Size", 0.0f, 1.0f, 0.5f));
    
    // Damping - controls high frequency rolloff
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        dampingID, "Damping", 0.0f, 1.0f, 0.5f));
    
    // Width - controls stereo width
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        widthID, "Width", 0.0f, 1.0f, 1.0f));
    
    // Wet level - controls the reverb effect amount
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        wetLevelID, "Wet Level", 0.0f, 1.0f, 0.33f));
    
    // Dry level - controls the amount of original signal
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        dryLevelID, "Dry Level", 0.0f, 1.0f, 0.4f));
    
    return { params.begin(), params.end() };
}

void CustomReverbAudioProcessor::updateReverbParameters()
{
    reverbParams.roomSize = *parameters.getRawParameterValue(roomSizeID);
    reverbParams.damping = *parameters.getRawParameterValue(dampingID);
    reverbParams.width = *parameters.getRawParameterValue(widthID);
    reverbParams.wetLevel = *parameters.getRawParameterValue(wetLevelID);
    reverbParams.dryLevel = *parameters.getRawParameterValue(dryLevelID);
    
    // Always freeze set to false (this would create an infinite reverb)
    reverbParams.freezeMode = 0.0f;
    
    // Apply the parameters to the reverb processor
    reverb.setParameters(reverbParams);
}

void CustomReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Update reverb parameters when any parameter changes
    updateReverbParameters();
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
    return 3.0; // Reverb tails can be long, so we give it a decent length
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
    // Initialize the reverb with the current sample rate
    reverb.reset();
    reverb.setSampleRate(sampleRate);
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

    // In case we have more outputs than inputs, clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Process the reverb - JUCE's reverb processor handles stereo processing internally
    // If the buffer is not stereo, we need to handle it differently
    if (totalNumInputChannels == 2 && totalNumOutputChannels == 2)
    {
        // Stereo processing - use JUCE's reverb processor
        reverb.processStereo(buffer.getWritePointer(0), 
                            buffer.getWritePointer(1), 
                            buffer.getNumSamples());
    }
    else if (totalNumInputChannels == 1 && totalNumOutputChannels == 1)
    {
        // Mono processing
        float* channelData = buffer.getWritePointer(0);
        
        // Create a duplicate buffer for stereo processing
        juce::AudioBuffer<float> stereoBuffer(2, buffer.getNumSamples());
        stereoBuffer.copyFrom(0, 0, channelData, buffer.getNumSamples());
        stereoBuffer.copyFrom(1, 0, channelData, buffer.getNumSamples());
        
        // Process the stereo buffer
        reverb.processStereo(stereoBuffer.getWritePointer(0), 
                            stereoBuffer.getWritePointer(1), 
                            buffer.getNumSamples());
        
        // Mix down to mono
        buffer.clear();
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            channelData[i] = (stereoBuffer.getSample(0, i) + stereoBuffer.getSample(1, i)) * 0.5f;
        }
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
    // Store the parameter state
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CustomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the parameter state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            
    // Make sure to update the reverb parameters after state is loaded
    updateReverbParameters();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
