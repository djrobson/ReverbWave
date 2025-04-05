#pragma once

#include <vector>

/**
 * Harmonic Detuning Interface
 * 
 * This header defines the interface for the harmonic detuning functionality,
 * which creates stereo enhancement by selectively detuning odd harmonics in the
 * left channel and even harmonics in the right channel.
 * 
 * The harmonic detuning effect works by:
 * 1. Separating the audio signal into frequency bands
 * 2. Processing odd-numbered harmonics slightly differently in the left channel
 * 3. Processing even-numbered harmonics slightly differently in the right channel
 * 4. The difference is subtle but creates a wider stereo image and more spatial depth
 * 
 * Technical approach:
 * - Uses gentle pitch-shifting on selected harmonics
 * - Maintains phase coherence to avoid comb filtering
 * - Preserves transients and attack characteristics
 * - Creates a natural-sounding stereo enhancement that works well with reverb
 * 
 * This type of processing is especially effective for:
 * - Adding width to mono sources
 * - Enhancing the stereo image of already stereo material
 * - Creating more immersive spatial effects when combined with reverb
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
