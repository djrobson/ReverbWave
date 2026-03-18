/*
  ==============================================================================

    DSPTests.cpp
    Created: 2025
    Author:  Claude Code

  ==============================================================================

  Unit tests for DSP algorithm operations in ReverbWave.

  This file tests the core digital signal processing algorithms:
  - Basic audio processing pipeline
  - Parameter response validation
  - Signal flow integrity
  - Edge case handling (silence, clipping, etc.)
  - Performance constraints

  These tests ensure that the DSP algorithms function correctly
  and maintain audio quality standards.
*/

#include "../../Source/PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
/**
 * Tests for DSP algorithm operations
 */
class DSPAlgorithmTests : public juce::UnitTest {
public:
  DSPAlgorithmTests() : UnitTest("DSP Algorithm Tests") {}

  void runTest() override {
    testSilentInputProducesSilentOutput();
    testParameterResponseValidation();
    testAudioProcessingPipeline();
    testEdgeCaseHandling();
    testSignalIntegrity();
    testProcessingPerformance();
  }

private:
  //==========================================================================
  void testSilentInputProducesSilentOutput() {
    beginTest("Silent input produces silent output");

    auto processor = createTestProcessor();
    auto buffer = createTestBuffer(512, 2); // 512 samples, stereo

    // Fill buffer with silence
    buffer.clear();

    // Process the silent buffer
    juce::MidiBuffer midiBuffer;
    processor->processBlock(buffer, midiBuffer);

    // Verify output is still silent
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      const float *channelData = buffer.getReadPointer(channel);
      for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        expect(channelData[sample] == 0.0f,
               "Silent input should produce silent output at channel " +
                   juce::String(channel) + ", sample " + juce::String(sample));
      }
    }
  }

  //==========================================================================
  void testParameterResponseValidation() {
    beginTest("Parameter response validation");

    auto processor = createTestProcessor();
    auto &apvts = processor->getAPVTS();

    // Test room size parameter response
    auto *roomSizeParam = apvts.getParameter("roomSize");
    if (roomSizeParam != nullptr) {
      // Set minimum room size
      roomSizeParam->setValueNotifyingHost(0.0f);
      juce::Thread::sleep(5); // Allow parameter update

      auto buffer1 = createTestBuffer(256, 2);
      fillWithTestTone(buffer1, 440.0f, 44100.0f);
      juce::MidiBuffer midiBuffer1;
      processor->processBlock(buffer1, midiBuffer1);

      // Set maximum room size
      roomSizeParam->setValueNotifyingHost(1.0f);
      juce::Thread::sleep(5); // Allow parameter update

      auto buffer2 = createTestBuffer(256, 2);
      fillWithTestTone(buffer2, 440.0f, 44100.0f);
      juce::MidiBuffer midiBuffer2;
      processor->processBlock(buffer2, midiBuffer2);

      // The two outputs should be different (different room sizes)
      bool outputsAreDifferent = false;
      for (int channel = 0; channel < 2; ++channel) {
        const float *data1 = buffer1.getReadPointer(channel);
        const float *data2 = buffer2.getReadPointer(channel);

        for (int sample = 0; sample < 256; ++sample) {
          if (std::abs(data1[sample] - data2[sample]) > 0.001f) {
            outputsAreDifferent = true;
            break;
          }
        }
        if (outputsAreDifferent)
          break;
      }

      expect(outputsAreDifferent,
             "Different room size settings should produce different outputs");
    }
  }

  //==========================================================================
  void testAudioProcessingPipeline() {
    beginTest("Audio processing pipeline");

    auto processor = createTestProcessor();
    auto buffer = createTestBuffer(1024, 2);

    // Fill with impulse signal (good for testing reverb)
    buffer.clear();
    buffer.setSample(0, 0, 1.0f); // Left channel impulse
    buffer.setSample(1, 0, 1.0f); // Right channel impulse

    // Process the buffer
    juce::MidiBuffer midiBuffer;
    processor->processBlock(buffer, midiBuffer);

    // Verify the impulse response has created reverb tail
    bool hasReverbTail = false;
    const float *leftData = buffer.getReadPointer(0);
    const float *rightData = buffer.getReadPointer(1);

    // Check for non-zero values after the initial impulse
    for (int sample = 10; sample < buffer.getNumSamples(); ++sample) {
      if (std::abs(leftData[sample]) > 0.001f ||
          std::abs(rightData[sample]) > 0.001f) {
        hasReverbTail = true;
        break;
      }
    }

    expect(hasReverbTail, "Impulse input should generate reverb tail");

    // Verify output is not clipped
    for (int channel = 0; channel < 2; ++channel) {
      const float *channelData = buffer.getReadPointer(channel);
      for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        expect(channelData[sample] >= -1.0f && channelData[sample] <= 1.0f,
               "Output should not exceed [-1.0, 1.0] range");
      }
    }
  }

  //==========================================================================
  void testEdgeCaseHandling() {
    beginTest("Edge case handling");

    auto processor = createTestProcessor();

    // Test with very small buffer size
    {
      auto smallBuffer = createTestBuffer(1, 2);
      fillWithTestTone(smallBuffer, 1000.0f, 44100.0f);
      juce::MidiBuffer midiBuffer;
      processor->processBlock(smallBuffer, midiBuffer);

      expect(true, "Should handle single sample buffer without crashing");
    }

    // Test with larger buffer size
    {
      auto largeBuffer = createTestBuffer(8192, 2);
      fillWithTestTone(largeBuffer, 440.0f, 44100.0f);
      juce::MidiBuffer midiBuffer;
      processor->processBlock(largeBuffer, midiBuffer);

      expect(true, "Should handle large buffer without crashing");
    }

    // Test with extreme parameter values
    auto &apvts = processor->getAPVTS();

    // Set all parameters to maximum
    const std::vector<std::string> paramIds = {
        "roomSize", "damping", "wetLevel", "dryLevel", "width", "freezeMode"};

    for (const auto &paramId : paramIds) {
      auto *param = apvts.getParameter(paramId);
      if (param != nullptr) {
        param->setValueNotifyingHost(1.0f);
      }
    }

    auto extremeBuffer = createTestBuffer(512, 2);
    fillWithTestTone(extremeBuffer, 440.0f, 44100.0f);
    juce::MidiBuffer midiBuffer;
    processor->processBlock(extremeBuffer, midiBuffer);

    // Verify no NaN or infinity values
    bool hasValidOutput = true;
    for (int channel = 0; channel < 2; ++channel) {
      const float *channelData = extremeBuffer.getReadPointer(channel);
      for (int sample = 0; sample < extremeBuffer.getNumSamples(); ++sample) {
        if (!std::isfinite(channelData[sample])) {
          hasValidOutput = false;
          break;
        }
      }
      if (!hasValidOutput)
        break;
    }

    expect(hasValidOutput,
           "Output should be finite even with extreme parameter values");
  }

  //==========================================================================
  void testSignalIntegrity() {
    beginTest("Signal integrity");

    auto processor = createTestProcessor();

    // Test frequency response with sine wave
    const float testFrequencies[] = {100.0f, 440.0f, 1000.0f, 4000.0f, 8000.0f};
    const int numFreqs = sizeof(testFrequencies) / sizeof(testFrequencies[0]);

    for (int freqIdx = 0; freqIdx < numFreqs; ++freqIdx) {
      auto buffer = createTestBuffer(2048, 2);
      fillWithTestTone(buffer, testFrequencies[freqIdx], 44100.0f);

      // Calculate RMS before processing
      float rmsInputL = calculateRMS(buffer, 0);
      float rmsInputR = calculateRMS(buffer, 1);

      juce::MidiBuffer midiBuffer;
      processor->processBlock(buffer, midiBuffer);

      // Calculate RMS after processing
      float rmsOutputL = calculateRMS(buffer, 0);
      float rmsOutputR = calculateRMS(buffer, 1);

      // The output should have some reasonable relationship to the input
      // (not necessarily equal due to reverb processing, but not zero or
      // infinite)
      expect(rmsOutputL > 0.001f && rmsOutputL < 10.0f * rmsInputL,
             "Left output RMS should be reasonable for " +
                 juce::String(testFrequencies[freqIdx]) + "Hz");

      expect(rmsOutputR > 0.001f && rmsOutputR < 10.0f * rmsInputR,
             "Right output RMS should be reasonable for " +
                 juce::String(testFrequencies[freqIdx]) + "Hz");
    }
  }

  //==========================================================================
  void testProcessingPerformance() {
    beginTest("Processing performance");

    auto processor = createTestProcessor();
    auto buffer = createTestBuffer(512, 2);
    fillWithTestTone(buffer, 440.0f, 44100.0f);

    // Measure processing time
    const int numIterations = 100;
    auto startTime = juce::Time::getMillisecondCounterHiRes();

    for (int i = 0; i < numIterations; ++i) {
      juce::MidiBuffer midiBuffer;
      processor->processBlock(buffer, midiBuffer);
    }

    auto endTime = juce::Time::getMillisecondCounterHiRes();
    double processingTime = endTime - startTime;
    double averageTime = processingTime / numIterations;

    // For 512 samples at 44.1kHz, we have about 11.6ms available
    // We should use much less than that to allow headroom
    double maxAllowedTime = 5.0; // 5ms should be plenty for 512 samples

    expect(averageTime < maxAllowedTime,
           "Processing should complete within real-time constraints. " +
               "Average time: " + juce::String(averageTime) + "ms, " +
               "Max allowed: " + juce::String(maxAllowedTime) + "ms");
  }

  //==========================================================================
  // Helper methods

  std::unique_ptr<CustomReverbAudioProcessor> createTestProcessor() {
    auto processor = std::make_unique<CustomReverbAudioProcessor>();
    processor->prepareToPlay(44100.0, 512);
    return processor;
  }

  juce::AudioBuffer<float> createTestBuffer(int numSamples, int numChannels) {
    return juce::AudioBuffer<float>(numChannels, numSamples);
  }

  void fillWithTestTone(juce::AudioBuffer<float> &buffer, float frequency,
                        float sampleRate) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      float *channelData = buffer.getWritePointer(channel);
      for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float phase = 2.0f * juce::MathConstants<float>::pi * frequency *
                      sample / sampleRate;
        channelData[sample] = 0.5f * std::sin(phase);
      }
    }
  }

  float calculateRMS(const juce::AudioBuffer<float> &buffer, int channel) {
    const float *channelData = buffer.getReadPointer(channel);
    float sum = 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      sum += channelData[sample] * channelData[sample];
    }

    return std::sqrt(sum / buffer.getNumSamples());
  }
};

// Register the test with JUCE's test system
static DSPAlgorithmTests dspTests;
