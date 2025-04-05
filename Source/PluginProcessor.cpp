
//==============================================================================
// This creates new instances of the plugin
// Make sure this is explicitly defined for VST3 compatibility
// Required for VST3 plugins to properly initialize
extern "C" JUCE_EXPORT juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomReverbAudioProcessor();
}
