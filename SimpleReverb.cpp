#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <memory>
#include <fstream>

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
        
        Parameters() {
            // Default settings
            roomSize = 0.5f;
            damping = 0.5f;
            wetLevel = 0.33f;
            dryLevel = 0.4f;
            width = 1.0f;
            freezeMode = 0.0f;
        }
    };
    
    SimpleReverb() {
        // Initialize with default parameters
        setParameters(Parameters());
        setSampleRate(44100.0f);
        reset();
    }
    
    // Set the reverb parameters
    void setParameters(const Parameters& params) {
        parameters = params;
        
        // Calculate internal coefficients from the parameters
        float roomScale = 0.28f + 0.5f * parameters.roomSize;
        float dampScale = 0.4f * parameters.damping;
        
        // Set the feedback coefficients based on room size
        for (int i = 0; i < numCombs; ++i) {
            feedbackCoeffs[i] = roomScale * combTuning[i] / 100.0f;
        }
        
        // Set the damping coefficients
        for (int i = 0; i < numCombs; ++i) {
            dampCoeffs[i] = dampScale;
        }
    }
    
    // Set the sample rate for the reverb
    void setSampleRate(float sampleRate) {
        this->sampleRate = sampleRate;
        
        // Resize the delay lines based on the sample rate
        float scaleFactor = sampleRate / 44100.0f;
        
        for (int i = 0; i < numCombs; ++i) {
            int delaySize = static_cast<int>(combTuning[i] * scaleFactor);
            combDelayLines[i].resize(delaySize, 0.0f);
            combIndices[i] = 0;
        }
        
        for (int i = 0; i < numAllPasses; ++i) {
            int delaySize = static_cast<int>(allpassTuning[i] * scaleFactor);
            allpassDelayLines[i].resize(delaySize, 0.0f);
            allpassIndices[i] = 0;
        }
    }
    
    // Reset all buffers
    void reset() {
        // Clear all delay lines
        for (int i = 0; i < numCombs; ++i) {
            std::fill(combDelayLines[i].begin(), combDelayLines[i].end(), 0.0f);
        }
        
        for (int i = 0; i < numAllPasses; ++i) {
            std::fill(allpassDelayLines[i].begin(), allpassDelayLines[i].end(), 0.0f);
        }
    }
    
    // Process a buffer of audio samples
    void processMono(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            float input = buffer[i];
            float output = 0.0f;
            
            // Process through the comb filters in parallel
            float combOut = 0.0f;
            for (int j = 0; j < numCombs; ++j) {
                combOut += processComb(j, input);
            }
            
            // Process through the allpass filters in series
            float allpassOut = combOut;
            for (int j = 0; j < numAllPasses; ++j) {
                allpassOut = processAllpass(j, allpassOut);
            }
            
            // Mix dry and wet signals
            output = parameters.dryLevel * input + parameters.wetLevel * allpassOut;
            
            // Write back to the buffer
            buffer[i] = output;
        }
    }
    
    // Process a stereo buffer of audio samples
    void processStereo(float* leftBuffer, float* rightBuffer, int numSamples) {
        const float spread = parameters.width;
        
        for (int i = 0; i < numSamples; ++i) {
            float inputL = leftBuffer[i];
            float inputR = rightBuffer[i];
            
            // Average the inputs for mono processing
            float monoInput = (inputL + inputR) * 0.5f;
            
            // Process through comb filters
            float combOutL = 0.0f;
            float combOutR = 0.0f;
            
            // Use the first half of the combs for the left channel, second half for right
            for (int j = 0; j < numCombs / 2; ++j) {
                combOutL += processComb(j, monoInput);
                combOutR += processComb(j + numCombs / 2, monoInput);
            }
            
            // Process through allpass filters (full chain for both channels)
            float allpassOutL = combOutL;
            float allpassOutR = combOutR;
            
            for (int j = 0; j < numAllPasses; ++j) {
                allpassOutL = processAllpass(j, allpassOutL);
                allpassOutR = processAllpass(j, allpassOutR);
            }
            
            // Apply stereo width
            float wetL = allpassOutL + (allpassOutR - allpassOutL) * (1.0f - spread);
            float wetR = allpassOutR + (allpassOutL - allpassOutR) * (1.0f - spread);
            
            // Mix dry and wet signals
            leftBuffer[i] = parameters.dryLevel * inputL + parameters.wetLevel * wetL;
            rightBuffer[i] = parameters.dryLevel * inputR + parameters.wetLevel * wetR;
        }
    }
    
private:
    // Process a sample through a comb filter
    float processComb(int index, float input) {
        float& delaySample = combDelayLines[index][combIndices[index]];
        
        // Get and store the delayed sample for output
        float output = delaySample;
        
        // Calculate the feedback with damping
        float feedback = output * (1.0f - dampCoeffs[index]) + previousCombs[index] * dampCoeffs[index];
        previousCombs[index] = feedback;
        
        // Apply feedback and input to the delay line
        delaySample = input + feedback * feedbackCoeffs[index] * (1.0f - parameters.freezeMode);
        
        // Update index
        combIndices[index] = (combIndices[index] + 1) % combDelayLines[index].size();
        
        return output;
    }
    
    // Process a sample through an allpass filter
    float processAllpass(int index, float input) {
        float& delaySample = allpassDelayLines[index][allpassIndices[index]];
        
        // Calculate output (direct + delay)
        float output = -input + delaySample;
        
        // Update delay line with input + feedback of the output
        delaySample = input + (output * 0.5f); // Constant fixed feedback for allpass
        
        // Update index
        allpassIndices[index] = (allpassIndices[index] + 1) % allpassDelayLines[index].size();
        
        return output;
    }
    
    // Parameters
    Parameters parameters;
    float sampleRate = 44100.0f;
    
    // Comb filters
    static const int numCombs = 8;
    std::vector<float> combDelayLines[numCombs];
    int combIndices[numCombs] = {0};
    float feedbackCoeffs[numCombs] = {0.0f};
    float dampCoeffs[numCombs] = {0.0f};
    float previousCombs[numCombs] = {0.0f};
    
    // These are the tuning values (in samples at 44.1kHz) for the delay lines
    int combTuning[numCombs] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
    
    // Allpass filters
    static const int numAllPasses = 4;
    std::vector<float> allpassDelayLines[numAllPasses];
    int allpassIndices[numAllPasses] = {0};
    
    // Allpass tuning values
    int allpassTuning[numAllPasses] = {556, 441, 341, 225};
};

class SimpleAudioProcessor {
public:
    SimpleAudioProcessor() {
        // Initialize the reverb with default settings
        reverb.setParameters(SimpleReverb::Parameters());
    }
    
    // Process a block of samples
    void processBlock(float* buffer, int numSamples, int numChannels) {
        if (numChannels == 1) {
            reverb.processMono(buffer, numSamples);
        } else if (numChannels == 2) {
            // Deinterleave stereo buffer (LRLRLR -> LLL, RRR)
            std::vector<float> leftChannel(numSamples);
            std::vector<float> rightChannel(numSamples);
            
            for (int i = 0; i < numSamples; ++i) {
                leftChannel[i] = buffer[i * 2];
                rightChannel[i] = buffer[i * 2 + 1];
            }
            
            // Process stereo data
            reverb.processStereo(leftChannel.data(), rightChannel.data(), numSamples);
            
            // Reinterleave (LLL, RRR -> LRLRLR)
            for (int i = 0; i < numSamples; ++i) {
                buffer[i * 2] = leftChannel[i];
                buffer[i * 2 + 1] = rightChannel[i];
            }
        }
    }
    
    // Set reverb parameters
    void setReverbParameters(const SimpleReverb::Parameters& params) {
        reverb.setParameters(params);
    }
    
    // Get the current parameters
    SimpleReverb::Parameters getReverbParameters() const {
        return parameters;
    }
    
    // Set sample rate
    void setSampleRate(float sampleRate) {
        reverb.setSampleRate(sampleRate);
    }
    
private:
    SimpleReverb reverb;
    SimpleReverb::Parameters parameters; // Current parameters
};

// Simple function to generate test audio (a sine wave)
std::vector<float> generateSineWave(float frequency, float sampleRate, float duration, int channels = 1) {
    int numSamples = static_cast<int>(sampleRate * duration);
    std::vector<float> buffer(numSamples * channels);
    
    for (int i = 0; i < numSamples; ++i) {
        float sample = std::sin(2.0f * M_PI * frequency * i / sampleRate);
        
        // Add to buffer (mono or interleaved stereo)
        if (channels == 1) {
            buffer[i] = sample;
        } else if (channels == 2) {
            buffer[i * 2] = sample;       // Left channel
            buffer[i * 2 + 1] = sample;   // Right channel
        }
    }
    
    return buffer;
}

// Simple WAV file writing function
bool writeWavFile(const std::string& filename, const std::vector<float>& buffer, 
                  int channels, float sampleRate) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    // WAV header
    struct WavHeader {
        char riff[4] = {'R', 'I', 'F', 'F'};
        uint32_t fileSize = 0;
        char wave[4] = {'W', 'A', 'V', 'E'};
        char fmt[4] = {'f', 'm', 't', ' '};
        uint32_t fmtChunkSize = 16;
        uint16_t audioFormat = 1; // PCM
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample = 16; // 16-bit
        char data[4] = {'d', 'a', 't', 'a'};
        uint32_t dataSize;
    };
    
    int numSamples = buffer.size() / channels;
    
    WavHeader header;
    header.numChannels = static_cast<uint16_t>(channels);
    header.sampleRate = static_cast<uint32_t>(sampleRate);
    header.bitsPerSample = 16;
    header.blockAlign = header.numChannels * (header.bitsPerSample / 8);
    header.byteRate = header.sampleRate * header.blockAlign;
    header.dataSize = numSamples * channels * (header.bitsPerSample / 8);
    header.fileSize = 36 + header.dataSize;
    
    // Write header
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    
    // Write audio data (convert float to 16-bit PCM)
    std::vector<int16_t> pcmData(buffer.size());
    for (size_t i = 0; i < buffer.size(); ++i) {
        // Convert float [-1,1] to int16 [-32768,32767]
        pcmData[i] = static_cast<int16_t>(std::max(-1.0f, std::min(buffer[i], 1.0f)) * 32767.0f);
    }
    
    file.write(reinterpret_cast<char*>(pcmData.data()), pcmData.size() * sizeof(int16_t));
    
    return true;
}

int main() {
    // Create our processor
    SimpleAudioProcessor processor;
    
    // Configure the processor
    float sampleRate = 44100.0f;
    processor.setSampleRate(sampleRate);
    
    // Set custom reverb parameters
    SimpleReverb::Parameters params;
    params.roomSize = 0.8f;    // Large room
    params.damping = 0.5f;     // Medium damping
    params.wetLevel = 0.6f;    // More wet signal
    params.dryLevel = 0.4f;    // Less dry signal
    params.width = 1.0f;       // Full stereo width
    params.freezeMode = 0.0f;  // No freeze
    processor.setReverbParameters(params);
    
    // Generate test signal (sine wave at 440 Hz)
    int channels = 2; // Stereo
    float duration = 3.0f; // 3 seconds
    std::vector<float> audioBuffer = generateSineWave(440.0f, sampleRate, duration, channels);
    
    // Create copy of the original signal for comparison
    std::vector<float> originalBuffer = audioBuffer;
    
    // Process the audio through our reverb
    processor.processBlock(audioBuffer.data(), audioBuffer.size() / channels, channels);
    
    // Write the processed audio to a file
    writeWavFile("reverb_output.wav", audioBuffer, channels, sampleRate);
    
    // Write the original audio to a file for comparison
    writeWavFile("original.wav", originalBuffer, channels, sampleRate);
    
    std::cout << "Reverb processing complete. Files generated:" << std::endl;
    std::cout << "1. original.wav - The original sine wave" << std::endl;
    std::cout << "2. reverb_output.wav - The sine wave with reverb applied" << std::endl;
    
    return 0;
}
