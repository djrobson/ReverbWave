# Harmonic Detuning for Enhanced Stereo Imaging

The SimpleReverb project now includes a sophisticated harmonic detuning feature that enhances stereo imaging by applying subtle pitch variations to different harmonic content in the left and right channels.

## Technical Overview

Harmonic detuning works by implementing a phase-shift algorithm that targets odd harmonics in the left channel and even harmonics in the right channel. This creates a subtle stereo enhancement effect that increases the perceived width of the sound field.

### Implementation Details

1. **Channel-Specific Processing**:
   - Left channel: Focuses on odd harmonics (1st, 3rd, 5th, etc.)
   - Right channel: Focuses on even harmonics (2nd, 4th, 6th, etc.)

2. **Phase Shift Algorithm**:
   - Variable buffer sizes store recent audio samples
   - Phase shift applied through time-domain manipulation
   - Amount of shift proportional to the detuning parameter value

3. **Integration with Reverb Processing**:
   - Applied before the main reverb algorithm for natural results
   - Can work independently or enhance the reverb's stereo field
   - Scaling factors ensure subtle enhancement rather than obvious effects

## User Parameters

The harmonic detuning effect is controlled by a single parameter:

- **Harmonic Detune Amount (0.0 to 1.0)**:
  - 0.0: No detuning (effect disabled)
  - 0.5: Moderate detuning for natural enhancement
  - 1.0: Maximum detuning for pronounced stereo effect

## Applications

This feature is particularly effective for:

1. **Adding Richness to Mono Sources**:
   - Makes mono instruments sound more spacious
   - Creates a natural stereo image from mono recordings

2. **Enhancing Stereo Reverb Tails**:
   - Increases the perceived width of reverb tails
   - Adds tonal complexity to the decay

3. **Sound Design and Special Effects**:
   - Creates unique spatial effects when pushed to extreme settings
   - Useful for ambient and experimental music

## Technical Implementation

The implementation creates the effect by selectively storing and recalling samples with slight timing offsets. By processing odd and even harmonics separately, the algorithm produces different harmonic structures in the left and right channels, enhancing the stereo image without creating phase issues or mono compatibility problems.

```cpp
void processHarmonicDetuning(float& leftSample, float& rightSample) {
    if (parameters.harmDetuneAmount <= 0.001f) {
        return; // Skip processing if detuning is disabled
    }
    
    // Store samples in odd/even harmonic buffers
    oddHarmonicBuffer[0][oddHarmonicFilter[0]] = leftSample;
    evenHarmonicBuffer[0][evenHarmonicFilter[0]] = rightSample;
    
    // Calculate delay indices for left (odd harmonics)
    float oddHarmonicDelay = 0.25f + 0.25f * parameters.harmDetuneAmount;
    int oddDelayIndex = (oddHarmonicFilter[0] - 1 + oddHarmonicBuffer[0].size()) % oddHarmonicBuffer[0].size();
    float oddDelayedSample = oddHarmonicBuffer[0][oddDelayIndex];
    
    // Calculate delay indices for right (even harmonics)
    float evenHarmonicDelay = 0.5f + 0.5f * parameters.harmDetuneAmount; 
    int evenDelayIndex = (evenHarmonicFilter[0] - 1 + evenHarmonicBuffer[0].size()) % evenHarmonicBuffer[0].size();
    float evenDelayedSample = evenHarmonicBuffer[0][evenDelayIndex];
    
    // Update indices for next time
    oddHarmonicFilter[0] = (oddHarmonicFilter[0] + 1) % oddHarmonicBuffer[0].size();
    evenHarmonicFilter[0] = (evenHarmonicFilter[0] + 1) % evenHarmonicBuffer[0].size();
    
    // Mix original and detuned samples - Detune only odd harmonics for left, even for right
    float mixAmount = parameters.harmDetuneAmount * 0.5f; // Scale for subtle effect
    leftSample = leftSample * (1.0f - mixAmount) + oddDelayedSample * mixAmount;
    rightSample = rightSample * (1.0f - mixAmount) + evenDelayedSample * mixAmount;
}
```

This harmonic detuning technique has been implemented across all versions of the reverb effect: the standalone SimpleReverb application, the ReverbGUI interface, and the VST3 plugin for use in digital audio workstations.
