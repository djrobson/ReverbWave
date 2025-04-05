#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <memory>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>

// Windows specific includes
#ifdef _WIN32
#define NOMINMAX
    #include <conio.h>    // For _kbhit() and _getch()
    #include <windows.h>  // For Windows specific functionality
#else
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif

#include "SpectrumAnalyzer.h"
#include "harmonic_detuning.h"

class SimpleReverb {
public:
    // Reverb parameters
    struct Parameters {
        float roomSize;     // 0.0 to 1.0
        float damping;      // 0.0 to 1.0
        float wetLevel;     // 0.0 to 1.0
        float dryLevel;     // 0.0 to 1.0
        float width;        // 0.0 to 1.0
        float freezeMode;   // 0.0 to 1.0
        float highFreqDelay; // 0.0 to 1.0, delay for upper harmonics
        float crossover;    // 0.0 to 1.0, crossover point between low/high freqs (0.5 = 1000Hz approx)
        float harmDetuneAmount; // 0.0 to 1.0, amount of detuning for odd/even harmonics in stereo
        
        Parameters() {
            // Default settings
            roomSize = 0.5f;
            damping = 0.5f;
            wetLevel = 0.33f;
            dryLevel = 0.4f;
            width = 1.0f;
            freezeMode = 0.0f;
            highFreqDelay = 0.3f;
            crossover = 0.5f;
            harmDetuneAmount = 0.0f; // Default to no detuning
        }
    };
    
    // Harmonic detuning variables
    static const int harmonicBufferSize = 50;
    std::vector<std::vector<float>> oddHarmonicBuffer;
    std::vector<std::vector<float>> evenHarmonicBuffer;
    std::vector<int> oddHarmonicFilter;
    std::vector<int> evenHarmonicFilter;
    
    // Public parameters
    Parameters parameters;
    float sampleRate;
    
    // Constructor
    SimpleReverb(float sampleRate = 44100.0f) 
        : sampleRate(sampleRate) {
        // Initialize parameters with defaults
        parameters = Parameters();
        
        // Initialize harmonic detuning buffers
        const int numChannels = 2;
        oddHarmonicBuffer.resize(numChannels);
        evenHarmonicBuffer.resize(numChannels);
        oddHarmonicFilter.resize(numChannels, 0);
        evenHarmonicFilter.resize(numChannels, 0);
        
        for (int i = 0; i < numChannels; ++i) {
            oddHarmonicBuffer[i].resize(harmonicBufferSize, 0.0f);
            evenHarmonicBuffer[i].resize(harmonicBufferSize, 0.0f);
        }
    }
    
    // Get the current parameters
    Parameters getParameters() const {
        return parameters;
    }
    
    // Set the parameters
    void setParameters(const Parameters& newParams) {
        parameters = newParams;
    }
    
    // Process harmonic detuning on stereo channels
    void processHarmonicDetuning(float& leftSample, float& rightSample) {
        // Call the external harmonic detuning implementation from harmonic_detuning.cpp
        ::processHarmonicDetuning(leftSample, rightSample, parameters.harmDetuneAmount);
    }
    
    // Rest of the SimpleReverb class methods...
};

// Main function and other code...
int main() {
    // Main functionality
    SimpleReverb reverb;
    
    // Set up harmonic detuning
    SimpleReverb::Parameters params = reverb.getParameters();
    params.harmDetuneAmount = 0.3f; // Set to 30% of maximum detuning
    reverb.setParameters(params);
    
    return 0;
}
