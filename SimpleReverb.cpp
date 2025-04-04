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
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif
#include "SpectrumAnalyzer.h"

// Windows specific includes
#ifdef _WIN32
    #include <conio.h>    // For _kbhit() and _getch()
    #include <windows.h>  // For Windows specific functionality
#endif
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif

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
        
        // Set high frequency delay parameters
        updateHighFreqDelay();
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
        
        // Initialize high-frequency delay lines
        for (int i = 0; i < numHighFreqDelays; ++i) {
            highFreqDelayLines[i].resize(maxHighFreqDelay * scaleFactor, 0.0f);
            highFreqIndices[i] = 0;
        }
        
        // Initialize high-pass and low-pass filters
        updateHighFreqDelay();
        
        // Initialize filter states
        for (int i = 0; i < 2; ++i) {
            lowpassState[i] = 0.0f;
            highpassState[i] = 0.0f;
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
        
        for (int i = 0; i < numHighFreqDelays; ++i) {
            std::fill(highFreqDelayLines[i].begin(), highFreqDelayLines[i].end(), 0.0f);
        }
        
        // Reset filter states
        for (int i = 0; i < 2; ++i) {
            lowpassState[i] = 0.0f;
            highpassState[i] = 0.0f;
        }
    }
    
    // Process a buffer of audio samples
    void processMono(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            float input = buffer[i];
            float output = 0.0f;
            
            // Split into low and high frequency bands
            float lowFreq, highFreq;
            splitFrequencies(input, lowFreq, highFreq, 0);
            
            // Process low frequencies through the comb filters in parallel
            float combOutLow = 0.0f;
            for (int j = 0; j < numCombs; ++j) {
                combOutLow += processComb(j, lowFreq);
            }
            
            // Process high frequencies with added delay
            float highFreqDelayed = processHighFreqDelay(highFreq, 0);
            
            // Process high frequencies through the comb filters
            float combOutHigh = 0.0f;
            for (int j = 0; j < numCombs; ++j) {
                combOutHigh += processComb(j, highFreqDelayed);
            }
            
            // Combine low and high frequency outputs
            float combOut = combOutLow + combOutHigh;
            
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
            
            // Split into low and high frequency bands for left channel
            float lowFreqL, highFreqL;
            splitFrequencies(monoInput, lowFreqL, highFreqL, 0);
            
            // Split into low and high frequency bands for right channel
            float lowFreqR, highFreqR;
            splitFrequencies(monoInput, lowFreqR, highFreqR, 1);
            
            // Process through comb filters
            float combOutLowL = 0.0f;
            float combOutLowR = 0.0f;
            float combOutHighL = 0.0f;
            float combOutHighR = 0.0f;
            
            // Process high frequencies with added delay
            float highFreqDelayedL = processHighFreqDelay(highFreqL, 0);
            float highFreqDelayedR = processHighFreqDelay(highFreqR, 1);
            
            // Use the first half of the combs for the left channel, second half for right
            for (int j = 0; j < numCombs / 2; ++j) {
                combOutLowL += processComb(j, lowFreqL);
                combOutLowR += processComb(j + numCombs / 2, lowFreqR);
                
                combOutHighL += processComb(j, highFreqDelayedL);
                combOutHighR += processComb(j + numCombs / 2, highFreqDelayedR);
            }
            
            // Combine low and high frequency outputs
            float combOutL = combOutLowL + combOutHighL;
            float combOutR = combOutLowR + combOutHighR;
            
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
    
    // Split signal into low and high frequency components
    void splitFrequencies(float input, float& lowOut, float& highOut, int channel) {
        // Simple one-pole low-pass filter
        lowOut = (1.0f - lowpassCoeff) * input + lowpassCoeff * lowpassState[channel];
        lowpassState[channel] = lowOut;
        
        // High frequencies are the remainder (input - low frequencies)
        highOut = input - lowOut;
    }
    
    // Add configurable delay to high frequencies
    float processHighFreqDelay(float input, int channel) {
        // Add the input to the delay line
        highFreqDelayLines[channel][highFreqIndices[channel]] = input;
        
        // Get the delayed output
        int delayLength = static_cast<int>(highFreqDelayAmount);
        int readIndex = (highFreqIndices[channel] - delayLength + highFreqDelayLines[channel].size()) % highFreqDelayLines[channel].size();
        float output = highFreqDelayLines[channel][readIndex];
        
        // Update index
        highFreqIndices[channel] = (highFreqIndices[channel] + 1) % highFreqDelayLines[channel].size();
        
        return output;
    }
    
    // Update high frequency delay parameters
    void updateHighFreqDelay() {
        // Calculate the crossover frequency coefficient (0.0 to 1.0 maps to ~100Hz to ~5000Hz)
        float crossoverFreq = 100.0f * std::pow(50.0f, parameters.crossover);
        lowpassCoeff = std::exp(-2.0f * M_PI * crossoverFreq / sampleRate);
        
        // Calculate the high frequency delay amount based on the parameter
        highFreqDelayAmount = static_cast<float>(maxHighFreqDelay) * parameters.highFreqDelay;
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
    
    // High frequency delay
    static const int numHighFreqDelays = 2; // For stereo
    static const int maxHighFreqDelay = 500; // Maximum delay in samples at 44.1kHz
    std::vector<float> highFreqDelayLines[numHighFreqDelays];
    int highFreqIndices[numHighFreqDelays] = {0};
    float highFreqDelayAmount = 0.0f;
    
    // Crossover filter
    float lowpassCoeff = 0.0f;
    float lowpassState[2] = {0.0f, 0.0f}; // For stereo
    float highpassState[2] = {0.0f, 0.0f}; // For stereo
};

class SimpleAudioProcessor {
public:
    SimpleAudioProcessor() : spectrumAnalyzer(std::make_unique<SpectrumAnalyzer>()) {
        // Initialize the reverb with default settings
        reverb.setParameters(SimpleReverb::Parameters());
    }
    
    // Process a block of samples
    void processBlock(float* buffer, int numSamples, int numChannels) {
        if (numChannels == 1) {
            reverb.processMono(buffer, numSamples);
            
            // Feed samples to the spectrum analyzer
            for (int i = 0; i < numSamples; ++i) {
                spectrumAnalyzer->pushSample(buffer[i]);
            }
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
                
                // Feed the mixed signal to the spectrum analyzer
                spectrumAnalyzer->pushSample((leftChannel[i] + rightChannel[i]) * 0.5f);
            }
        }
        
        // Update the spectrum analyzer
        spectrumAnalyzer->update();
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
    
    // Get the spectrum analyzer
    SpectrumAnalyzer* getSpectrumAnalyzer() {
        return spectrumAnalyzer.get();
    }
    
private:
    SimpleReverb reverb;
    SimpleReverb::Parameters parameters; // Current parameters
    std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer;
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

// Utility to clear screen
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void initTerminal() {
    #ifdef _WIN32
        // Enable virtual terminal processing for color support on Windows
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    #endif
}

// Draw a spectrum analyzer visualization in the terminal
void drawSpectrumAnalyzer(SpectrumAnalyzer* analyzer, bool interactive = false) {
    // Terminal dimensions (adjust as needed)
    const int width = 80;
    const int height = 20;
    
    // Buffer for the visualization
    std::vector<std::string> buffer;
    
    // Update and draw the spectrum
    analyzer->update();
    analyzer->draw(buffer, width, height);
    
    // Display
    clearScreen();
    
    // Print header
    std::cout << "=== Real-Time Spectrum Analyzer with Fluid Wave Animations ===" << std::endl;
    
    // Print the visualization
    for (const auto& line : buffer) {
        std::cout << line << std::endl;
    }
    
    // Print controls if in interactive mode
    if (interactive) {
        std::cout << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  'M' - Change Animation Mode (current: ";
        switch (analyzer->getAnimationMode()) {
            case 0: std::cout << "Wave"; break;
            case 1: std::cout << "Bars"; break;
            case 2: std::cout << "Particles"; break;
        }
        std::cout << ")" << std::endl;
        
        std::cout << "  'C' - Change Color Scheme (current: ";
        switch (analyzer->getColorScheme()) {
            case 0: std::cout << "Blue"; break;
            case 1: std::cout << "Purple"; break;
            case 2: std::cout << "Green"; break;
        }
        std::cout << ")" << std::endl;
        
        std::cout << "  'Q' - Quit" << std::endl;
    }
}

int main() {
    // Initialize terminal for cross-platform color support
    initTerminal();
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
    params.highFreqDelay = 0.4f; // Medium delay for high frequencies
    params.crossover = 0.6f;   // Crossover at ~1.5kHz
    processor.setReverbParameters(params);
    
    // Generate test signals (different frequencies for more interesting visualization)
    int channels = 2; // Stereo
    float duration = 3.0f; // 3 seconds
    
    // Mix multiple sine waves for a richer spectrum
    std::vector<float> audioBuffer(static_cast<int>(sampleRate * duration) * channels, 0.0f);
    std::vector<float> freqs = {110.0f, 220.0f, 440.0f, 880.0f, 1760.0f};
    std::vector<float> amps = {0.3f, 0.25f, 0.2f, 0.15f, 0.1f};
    
    for (int i = 0; i < freqs.size(); ++i) {
        auto sineWave = generateSineWave(freqs[i], sampleRate, duration, channels);
        for (size_t j = 0; j < audioBuffer.size(); ++j) {
            audioBuffer[j] += sineWave[j] * amps[i];
        }
    }
    
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
    
    // Enter interactive demo mode
    std::cout << "\nStarting Real-Time Spectrum Analyzer Demo...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Get the spectrum analyzer
    auto analyzer = processor.getSpectrumAnalyzer();
    
    // Interactive mode flag
    bool interactive = true;
    bool running = true;
    
    // Main demo loop
    while (running) {
        // Draw the spectrum analyzer
        drawSpectrumAnalyzer(analyzer, interactive);
        
        // Process chunk of audio continuously (repeating for visualization)
        int blockSize = 1024;
        static int position = 0;
        
        for (int i = 0; i < blockSize; ++i) {
            int idx = (position + i) % (audioBuffer.size() / channels);
            for (int c = 0; c < channels; ++c) {
                // Feed samples to analyzer
                if (c == 0) { // Just use left channel for analyzer
                    analyzer->pushSample(audioBuffer[idx * channels + c]);
                }
            }
        }
        position = (position + blockSize) % (audioBuffer.size() / channels);
        
        // Handle user input in interactive mode
        if (interactive) {
            // Non-blocking input check (platform dependent)
            #ifdef _WIN32
                if (_kbhit()) {
                    char key = tolower(_getch());
                    if (key == 'q') {
                        running = false;
                    } else if (key == 'm') {
                        analyzer->setAnimationMode((analyzer->getAnimationMode() + 1) % 3);
                    } else if (key == 'c') {
                        analyzer->setColorScheme((analyzer->getColorScheme() + 1) % 3);
                    }
                }
            #else
                // For Unix-like systems, implement non-blocking input with ncurses or similar approach
                // This is a simplified approach - for production code, consider using ncurses or similar libraries
                struct timeval tv;
                fd_set fds;
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                FD_ZERO(&fds);
                FD_SET(STDIN_FILENO, &fds);
                
                if (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0) {
                    char key;
                    if (read(STDIN_FILENO, &key, 1) > 0) {
                        key = tolower(key);
                        if (key == 'q') {
                            running = false;
                        } else if (key == 'm') {
                            analyzer->setAnimationMode((analyzer->getAnimationMode() + 1) % 3);
                        } else if (key == 'c') {
                            analyzer->setColorScheme((analyzer->getColorScheme() + 1) % 3);
                        }
                    }
                }
                
                // Also allow for automatic mode changes for demos
                static int counter = 0;
                if (++counter % 100 == 0) {
                    analyzer->setAnimationMode((analyzer->getAnimationMode() + 1) % 3);
                }
                if (counter % 200 == 0) {
                    analyzer->setColorScheme((analyzer->getColorScheme() + 1) % 3);
                }
            #endif
        }
        
        // Slow down the loop a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30fps
    }
    
    std::cout << "\nSpectrum Analyzer Demo complete.\n";
    
    return 0;
}
