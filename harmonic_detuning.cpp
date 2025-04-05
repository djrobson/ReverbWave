#include <vector>
#include <cmath>

/**
 * Harmonic Detuning Implementation
 * 
 * This file contains the implementation of the harmonic detuning functionality,
 * which creates stereo enhancement by selectively detuning odd harmonics in the
 * left channel and even harmonics in the right channel.
 */

// Structure for harmonic detuning parameters
struct HarmonicDetuningParams {
    float amount;         // 0.0 to 1.0, amount of detuning effect
    float mix;            // Mix level for the detuned signal
    float sampleRate;     // Current sample rate
};

// Buffer sizes and state information
static const int HARMONIC_BUFFER_SIZE = 50;
static std::vector<std::vector<float>> oddHarmonicBuffer(2, std::vector<float>(HARMONIC_BUFFER_SIZE, 0.0f));
static std::vector<std::vector<float>> evenHarmonicBuffer(2, std::vector<float>(HARMONIC_BUFFER_SIZE, 0.0f));
static std::vector<int> oddHarmonicFilter(2, 0);
static std::vector<int> evenHarmonicFilter(2, 0);

/**
 * Process stereo audio with harmonic detuning effect
 * 
 * @param leftSample  Reference to left channel sample
 * @param rightSample Reference to right channel sample
 * @param params      Detuning parameters
 */
void processHarmonicDetuning(float& leftSample, float& rightSample, const HarmonicDetuningParams& params) {
    // Skip processing if detuning is disabled
    if (params.amount <= 0.001f) {
        return;
    }
    
    // Store samples in odd/even harmonic buffers
    oddHarmonicBuffer[0][oddHarmonicFilter[0]] = leftSample;
    evenHarmonicBuffer[0][evenHarmonicFilter[0]] = rightSample;
    
    // Calculate delay indices for left (odd harmonics)
    float oddHarmonicDelay = 0.25f + 0.25f * params.amount;
    int oddDelayIndex = (oddHarmonicFilter[0] - 1 + oddHarmonicBuffer[0].size()) % oddHarmonicBuffer[0].size();
    float oddDelayedSample = oddHarmonicBuffer[0][oddDelayIndex];
    
    // Calculate delay indices for right (even harmonics)
    float evenHarmonicDelay = 0.5f + 0.5f * params.amount; 
    int evenDelayIndex = (evenHarmonicFilter[0] - 1 + evenHarmonicBuffer[0].size()) % evenHarmonicBuffer[0].size();
    float evenDelayedSample = evenHarmonicBuffer[0][evenDelayIndex];
    
    // Update indices for next time
    oddHarmonicFilter[0] = (oddHarmonicFilter[0] + 1) % oddHarmonicBuffer[0].size();
    evenHarmonicFilter[0] = (evenHarmonicFilter[0] + 1) % evenHarmonicBuffer[0].size();
    
    // Mix original and detuned samples - Detune only odd harmonics for left, even for right
    float mixAmount = params.amount * params.mix; // Scale for subtle effect
    leftSample = leftSample * (1.0f - mixAmount) + oddDelayedSample * mixAmount;
    rightSample = rightSample * (1.0f - mixAmount) + evenDelayedSample * mixAmount;
}

/**
 * Reset all harmonic detuning buffers
 */
void resetHarmonicDetuning() {
    for (int i = 0; i < 2; ++i) {
        std::fill(oddHarmonicBuffer[i].begin(), oddHarmonicBuffer[i].end(), 0.0f);
        std::fill(evenHarmonicBuffer[i].begin(), evenHarmonicBuffer[i].end(), 0.0f);
        oddHarmonicFilter[i] = 0;
        evenHarmonicFilter[i] = 0;
    }
}

/**
 * Simplified interface for standard detuning processing
 * 
 * @param leftSample  Reference to left channel sample
 * @param rightSample Reference to right channel sample
 * @param amount      Amount of detuning (0.0 to 1.0)
 */
void processHarmonicDetuning(float& leftSample, float& rightSample, float amount) {
    HarmonicDetuningParams params = {
        amount,    // Amount of detuning
        0.5f,      // Default mix level 
        44100.0f   // Default sample rate
    };
    
    processHarmonicDetuning(leftSample, rightSample, params);
}
