#pragma once

#include <vector>

/**
 * Harmonic Detuning Interface
 * 
 * This header defines the interface for the harmonic detuning functionality,
 * which creates stereo enhancement by selectively detuning odd harmonics in the
 * left channel and even harmonics in the right channel.
 */

// Structure for harmonic detuning parameters
struct HarmonicDetuningParams {
    float amount;         // 0.0 to 1.0, amount of detuning effect
    float mix;            // Mix level for the detuned signal
    float sampleRate;     // Current sample rate
};

/**
 * Process stereo audio with harmonic detuning effect
 * 
 * @param leftSample  Reference to left channel sample
 * @param rightSample Reference to right channel sample
 * @param params      Detuning parameters
 */
void processHarmonicDetuning(float& leftSample, float& rightSample, const HarmonicDetuningParams& params);

/**
 * Reset all harmonic detuning buffers
 */
void resetHarmonicDetuning();

/**
 * Simplified interface for standard detuning processing
 * 
 * @param leftSample  Reference to left channel sample
 * @param rightSample Reference to right channel sample
 * @param amount      Amount of detuning (0.0 to 1.0)
 */
void processHarmonicDetuning(float& leftSample, float& rightSample, float amount);
