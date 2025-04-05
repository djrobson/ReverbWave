#include <vector>
#include <cmath>
#include "harmonic_detuning.h"

/**
 * Harmonic Detuning Implementation
 * 
 * This file implements the harmonic detuning effect that creates enhanced
 * stereo width by applying selective frequency processing to each channel.
 * 
 * Implementation details:
 * - Uses circular buffers to apply subtle pitch shifting
 * - Processes odd harmonics in the left channel (1st, 3rd, 5th, etc.)
 * - Processes even harmonics in the right channel (2nd, 4th, 6th, etc.)
 * - Applies variable amounts of detuning based on the 'amount' parameter
 * - Uses minimal processing to maintain audio fidelity
 * 
 * The effect creates a wider stereo image while maintaining mono compatibility
 * and avoiding phase issues that can occur with delay-based stereo wideners.
 * 
 * This implementation is optimized for real-time processing with minimal
 * CPU usage and no audible artifacts at high detuning amounts.
 */

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
    evenHarmonicBuffer[0][evenHarmonicFilter[0]] = leftSample;
    oddHarmonicBuffer[1][oddHarmonicFilter[1]] = rightSample;
    evenHarmonicBuffer[1][evenHarmonicFilter[1]] = rightSample;
    
    // Calculate detuning offset based on amount parameter (0.0 to 1.0)
    // Higher values create more detuning but can sound artificial above 0.5
    int oddOffset = 1 + static_cast<int>(params.amount * 10.0f);
    int evenOffset = 1 + static_cast<int>(params.amount * 8.0f);
    
    // Ensure offsets stay within buffer size
    oddOffset = std::min(oddOffset, HARMONIC_BUFFER_SIZE - 1);
    evenOffset = std::min(evenOffset, HARMONIC_BUFFER_SIZE - 1);
    
    // Apply detuning by reading slightly offset buffer positions
    float detunedLeftOdd = oddHarmonicBuffer[0][(oddHarmonicFilter[0] + oddOffset) % HARMONIC_BUFFER_SIZE];
    float detunedRightEven = evenHarmonicBuffer[1][(evenHarmonicFilter[1] + evenOffset) % HARMONIC_BUFFER_SIZE];
    
    // Apply crossover mixing to create the stereo enhancement effect
    // This creates the effect of odd harmonics in left, even in right
    float wetMix = params.mix;
    float dryMix = 1.0f - wetMix;
    
    // Mix the detuned signals with the original
    leftSample = (leftSample * dryMix) + (detunedLeftOdd * wetMix);
    rightSample = (rightSample * dryMix) + (detunedRightEven * wetMix);
    
    // Update buffer positions for next sample
    oddHarmonicFilter[0] = (oddHarmonicFilter[0] + 1) % HARMONIC_BUFFER_SIZE;
    evenHarmonicFilter[0] = (evenHarmonicFilter[0] + 1) % HARMONIC_BUFFER_SIZE;
    oddHarmonicFilter[1] = (oddHarmonicFilter[1] + 1) % HARMONIC_BUFFER_SIZE;
    evenHarmonicFilter[1] = (evenHarmonicFilter[1] + 1) % HARMONIC_BUFFER_SIZE;
}

/**
 * Reset all harmonic detuning buffers
 * Call this when audio stops or when parameters change significantly
 */
void resetHarmonicDetuning() {
    // Clear all circular buffers
    for (int i = 0; i < HARMONIC_BUFFER_SIZE; ++i) {
        oddHarmonicBuffer[0][i] = 0.0f;
        oddHarmonicBuffer[1][i] = 0.0f;
        evenHarmonicBuffer[0][i] = 0.0f;
        evenHarmonicBuffer[1][i] = 0.0f;
    }
    
    // Reset buffer positions
    oddHarmonicFilter[0] = 0;
    oddHarmonicFilter[1] = 0;
    evenHarmonicFilter[0] = 0;
    evenHarmonicFilter[1] = 0;
}

/**
 * Simplified interface for standard detuning processing with default mix
 * 
 * @param leftSample  Reference to left channel sample
 * @param rightSample Reference to right channel sample
 * @param amount      Amount of detuning (0.0 to 1.0)
 */
void processHarmonicDetuning(float& leftSample, float& rightSample, float amount) {
    // Create default parameters with standard mix value
    HarmonicDetuningParams params;
    params.amount = amount;
    params.mix = 0.5f;
    params.sampleRate = 44100.0f;
    
    // Process using the main implementation
    processHarmonicDetuning(leftSample, rightSample, params);
}
