// Process harmonic detuning on stereo channels
void processHarmonicDetuning(float& leftSample, float& rightSample) {
    if (parameters.harmDetuneAmount <= 0.001f) {
        return; // Skip processing if detuning is disabled
    }
    
    // Get the detune amount (0-1 maps to 0-10 Hz shift)
    float detuneAmount = parameters.harmDetuneAmount * 10.0f;
    
    // Store samples in odd/even harmonic buffers
    oddHarmonicBufferL[oddHarmonicPos] = leftSample;
    evenHarmonicBufferR[evenHarmonicPos] = rightSample;
    
    // Calculate the phase shift amount for the sample rate
    float phaseShiftSamples = detuneAmount / sampleRate * harmonicBufferSize;
    
    // Detune odd harmonics in left channel
    int readPos = oddHarmonicPos - static_cast<int>(phaseShiftSamples) % harmonicBufferSize;
    if (readPos < 0)
        readPos += harmonicBufferSize;
    leftSample = oddHarmonicBufferL[readPos];
    
    // Detune even harmonics in right channel (opposite direction)
    readPos = evenHarmonicPos + static_cast<int>(phaseShiftSamples) % harmonicBufferSize;
    if (readPos >= harmonicBufferSize)
        readPos -= harmonicBufferSize;
    rightSample = evenHarmonicBufferR[readPos];
    
    // Update buffer positions
    oddHarmonicPos = (oddHarmonicPos + 1) % harmonicBufferSize;
    evenHarmonicPos = (evenHarmonicPos + 1) % harmonicBufferSize;
}
