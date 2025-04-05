/*
  ==============================================================================

    CustomReverbProcessor.h
    Created: 2023
    Author:  Audio Developer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * Forward declaration for spectrum analyzer component
 * This allows the processor to send FFT data to the visual analyzer
 */
class SpectrumAnalyzerComponent;

//==============================================================================
/**
 * CustomReverbAudioProcessor
 * 
 * This is the main audio processor class for the VST plugin.
 * It handles audio processing, parameter management, and DSP operations including:
 * - Room reverberation with adjustable parameters
 * - Stereo width enhancement through harmonic detuning
 * - High frequency delay for natural sound decay
 * - Freeze mode for infinite sustain
 * - Real-time spectrum analysis and visualization
 * 
 * Implements AudioProcessor for audio handling and ValueTreeState::Listener for parameter updates
 */
class CustomReverbAudioProcessor  : public juce::AudioProcessor,
                                   public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    /** Constructor - initializes parameters and DSP objects */
    CustomReverbAudioProcessor();
    
    /** Destructor - cleans up allocated resources */
    ~CustomReverbAudioProcessor() override;

    //==============================================================================
    /** Prepares the processor for playback with given sample rate and block size */
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    /** Releases resources when playback stops */
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    /** Validates if the current bus layout is supported by the processor */
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /** Core audio processing method - applies reverb and effects to each buffer */
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    /** Creates the processor's GUI editor component */
    juce::AudioProcessorEditor* createEditor() override;
    
    /** Returns whether the processor has a GUI editor */
    bool hasEditor() const override;

    //==============================================================================
    /** Returns the name of the processor */
    const juce::String getName() const override;

    /** Returns whether the processor accepts MIDI input */
    bool acceptsMidi() const override;
    
    /** Returns whether the processor produces MIDI output */
    bool producesMidi() const override;
    
    /** Returns whether the processor is MIDI effect (not audio) */
    bool isMidiEffect() const override;
    
    /** Returns the tail length in seconds (reverb decay time) */
    double getTailLengthSeconds() const override;

    //==============================================================================
    /** Returns the number of preset programs */
    int getNumPrograms() override;
    
    /** Returns the index of the current program */
    int getCurrentProgram() override;
    
    /** Sets the current program by index */
    void setCurrentProgram (int index) override;
    
    /** Gets the name of a preset program by index */
    const juce::String getProgramName (int index) override;
    
    /** Changes the name of a preset program */
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    /** Saves processor state to memory block for DAW session storage */
    void getStateInformation (juce::MemoryBlock& destData) override;
    
    /** Restores processor state from memory block when DAW session is loaded */
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    /** Called when a parameter value changes - updates internal DSP state */
    void parameterChanged(const juce::String &parameterID, float newValue) override;
    
    /** Returns a reference to the parameter tree for editor access */
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    /** Sets the spectrum analyzer component reference for FFT data visualization */
    void setSpectrumAnalyzer(SpectrumAnalyzerComponent* analyzer);
    
    /** Constants for FFT analysis */
    enum
    {
        fftOrder = 11,               // 2048 samples for FFT (2^11)
        fftSize  = 1 << fftOrder,    // Size based on the order
        scopeSize = 512              // Number of points to display in visualizer
    };

private:
    //==============================================================================
    /**
     * Custom reverb parameters structure
     * 
     * Extends the standard JUCE reverb parameters with additional
     * controls for high frequency processing and harmonic detuning
     */
    struct CustomReverbParameters {
        float roomSize = 0.5f;        // Controls the size of the simulated room (0.0=small, 1.0=large)
        float damping = 0.5f;         // Controls high frequency absorption (0.0=bright, 1.0=dark)
        float wetLevel = 0.33f;       // Amount of processed signal in output (0.0=dry, 1.0=wet)
        float dryLevel = 0.4f;        // Amount of unprocessed signal in output (0.0=none, 1.0=full)
        float width = 1.0f;           // Stereo width enhancement (0.0=mono, 1.0=wide)
        float freezeMode = 0.0f;      // Infinite reverb tail when 1.0 (0.0=normal, 1.0=freeze)
        float highFreqDelay = 0.3f;   // Separate delay for high frequencies (0.0=same as low, 1.0=max delay)
        float crossover = 0.5f;       // Frequency split point between low/high bands (0.5≈1000Hz)
        float harmDetuneAmount = 0.0f; // Stereo enhancement via harmonic detuning (0.0=none, 1.0=maximum)
    };
    
    /** JUCE built-in reverb processor and parameters */
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    
    /** Custom extended parameters for our enhanced reverb features */
    CustomReverbParameters customParams;
    
    /** Parameter management tree - stores all adjustable parameters */
    juce::AudioProcessorValueTreeState apvts;
    
    /** Creates the parameter layout and ranges for the AudioProcessorValueTreeState */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    /** Updates the reverb parameters based on current values in the parameter tree */
    void updateReverbParameters();
    
    //==============================================================================
    // High Frequency Delay Implementation
    
    /** Delay buffers for high frequency content (separated for stereo channels) */
    std::vector<float> highFreqDelayBufferL;
    std::vector<float> highFreqDelayBufferR;
    
    /** Configuration for high frequency delay processing */
    int highFreqBufferSize = 0;      // Size of the delay buffer in samples
    int highFreqDelayReadPos = 0;    // Current read position in the delay buffer
    int highFreqDelayWritePos = 0;   // Current write position in the delay buffer
    float highFreqDelayAmount = 0.0f; // Amount of delay to apply (0.0 to 1.0)
    
    /** Low pass filter coefficients and state for the crossover filter */
    float lowpassCoeff = 0.0f;      // Filter coefficient (cutoff control)
    float lowpassStateL = 0.0f;     // Filter state for left channel
    float lowpassStateR = 0.0f;     // Filter state for right channel
    
    //==============================================================================
    // Harmonic Detuning Implementation
    
    /** Maximum size of harmonic detuning delay buffer */
    static const int maxHarmonicFilterSize = 50;
    
    /** Buffers for storing audio for harmonic detuning effect */
    std::vector<float> oddHarmonicBufferL;  // Buffer for odd harmonics (left channel)
    std::vector<float> evenHarmonicBufferR; // Buffer for even harmonics (right channel)
    
    /** Current positions in harmonic buffers */
    int oddHarmonicPos = 0;   // Position in the odd harmonics buffer
    int evenHarmonicPos = 0;  // Position in the even harmonics buffer
    
    //==============================================================================
    // DSP Processing Methods
    
    /** Splits audio into low and high frequency bands using a simple one-pole filter */
    void splitFrequencies(float input, float& lowOut, float& highOut, float& state);
    
    /** Processes the harmonic detuning effect on stereo channels */
    void processHarmonicDetuning(float& leftSample, float& rightSample);
    
    /** Processes the high frequency delay effect on a single channel */
    float processHighFreqDelay(float input, std::vector<float>& buffer);
    
    /** Updates parameters for high frequency processing based on current values */
    void updateHighFreqParameters();
    
    //==============================================================================
    // Spectrum Analysis Implementation
    
    /** Pointer to the spectrum analyzer component for visualization */
    SpectrumAnalyzerComponent* spectrumAnalyzer = nullptr;
    
    /** FFT analysis objects for spectrum visualization */
    juce::dsp::FFT forwardFFT;                    // FFT processor
    juce::dsp::WindowingFunction<float> window;   // Window function to reduce spectral leakage
    
    /** FFT data storage and state */
    float fifo[fftSize];           // Buffer for collecting samples for FFT
    float fftData[2 * fftSize];    // Buffer for FFT results (complex values)
    int fifoIndex = 0;             // Current position in the fifo buffer
    bool nextFFTBlockReady = false; // Flag indicating when FFT block is ready to process
    float scopeData[scopeSize];    // Processed data ready for visualization
    
    /** FFT processing methods */
    void pushNextSampleIntoFifo(float sample) noexcept;  // Adds a sample to the FFT buffer
    void drawNextFrameOfSpectrum();                      // Triggers visualization update
    void calculateFrequencySpectrum();                   // Performs FFT and prepares data for display
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomReverbAudioProcessor)
};
