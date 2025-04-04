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
       apvts(*this, nullptr, "Parameters", createParameters())
{
    // Initialize reverb parameters with defaults
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.33f;
    reverbParams.dryLevel = 0.4f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    
    // Set the initial reverb parameters
    reverb.setParameters(reverbParams);
    
    // Add listeners for all parameters
    apvts.addParameterListener("roomSize", this);
    apvts.addParameterListener("damping", this);
    apvts.addParameterListener("wetLevel", this);
    apvts.addParameterListener("dryLevel", this);
    apvts.addParameterListener("width", this);
    apvts.addParameterListener("freezeMode", this);
}

CustomReverbAudioProcessor::~CustomReverbAudioProcessor()
{
    // Remove listeners for all parameters
    apvts.removeParameterListener("roomSize", this);
    apvts.removeParameterListener("damping", this);
    apvts.removeParameterListener("wetLevel", this);
    apvts.removeParameterListener("dryLevel", this);
    apvts.removeParameterListener("width", this);
    apvts.removeParameterListener("freezeMode", this);
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
    return 2.0; // A reasonable tail time for a reverb effect
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
    juce::ignoreUnused(index);
}

const juce::String CustomReverbAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void CustomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void CustomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize the reverb with the current sample rate
    reverb.reset();
    reverb.setSampleRate(sampleRate);
    
    // Update parameters with current APVTS values
    updateReverbParameters();
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
    // We support mono and stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Check if input and output channel configurations match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
  #endif
}
#endif

void CustomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Apply reverb to the buffer
    // The JUCE reverb processor can handle both mono and stereo signals
    if (totalNumInputChannels == 1)
    {
        // For mono input
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
    }
    else
    {
        // For stereo input
        reverb.processStereo(buffer.getWritePointer(0), 
                            buffer.getWritePointer(1), 
                            buffer.getNumSamples());
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
    // Store the APVTS state to save plugin parameters
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CustomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the APVTS state to load plugin parameters
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    
    // Update the reverb parameters based on loaded state
    updateReverbParameters();
}

//==============================================================================
void CustomReverbAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    // Update the relevant reverb parameter when a control changes
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
    
    // Apply the updated parameters
    reverb.setParameters(reverbParams);
}

void CustomReverbAudioProcessor::updateReverbParameters()
{
    // Get current parameter values from the APVTS
    reverbParams.roomSize = *apvts.getRawParameterValue("roomSize");
    reverbParams.damping = *apvts.getRawParameterValue("damping");
    reverbParams.wetLevel = *apvts.getRawParameterValue("wetLevel");
    reverbParams.dryLevel = *apvts.getRawParameterValue("dryLevel");
    reverbParams.width = *apvts.getRawParameterValue("width");
    reverbParams.freezeMode = *apvts.getRawParameterValue("freezeMode");
    
    // Apply the updated parameters
    reverb.setParameters(reverbParams);
}

juce::AudioProcessorValueTreeState::ParameterLayout CustomReverbAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Add all the reverb parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("damping", "Damping", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("wetLevel", "Wet Level", 0.0f, 1.0f, 0.33f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("dryLevel", "Dry Level", 0.0f, 1.0f, 0.4f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("width", "Width", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("freezeMode", "Freeze Mode", 0.0f, 1.0f, 0.0f));
    
    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
