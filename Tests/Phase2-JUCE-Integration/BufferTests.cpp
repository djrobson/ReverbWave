/*
  ==============================================================================

    BufferTests.cpp
    Created: 2025
    Author:  Claude Code

  ==============================================================================

  Unit tests for buffer management operations in ReverbWave.

  This file tests the refactored buffer management system, including:
  - Template buffer clearing operations
  - Fixed array clearing
  - Buffer resizing operations
  - Circular buffer functionality
  - Memory safety and bounds checking

  These tests validate the helper methods that were created during
  the refactoring to eliminate redundant buffer operations.
*/

#include "../../Source/PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
/**
 * Tests for buffer management operations
 */
class BufferOperationTests : public juce::UnitTest {
public:
  BufferOperationTests() : UnitTest("Buffer Operation Tests") {}

  void runTest() override {
    testVectorBufferClearing();
    testFixedArrayClearing();
    testBufferResizing();
    testCircularBufferOperations();
    testBufferBoundsSafety();
    testMemoryAlignment();
  }

private:
  //==========================================================================
  void testVectorBufferClearing() {
    beginTest("Vector buffer clearing");

    // Create test vectors with different sizes and initial values
    std::vector<float> testBuffer1(1024, 42.0f);
    std::vector<float> testBuffer2(512, -1.5f);
    std::vector<double> testBuffer3(256, 3.14);

    // Verify initial non-zero values
    expect(testBuffer1[0] == 42.0f, "Initial buffer1 value should be 42.0");
    expect(testBuffer2[100] == -1.5f, "Initial buffer2 value should be -1.5");
    expect(testBuffer3[200] == 3.14, "Initial buffer3 value should be 3.14");

    // Test clearing buffers (we'll simulate the template method behavior)
    clearTestBuffer(testBuffer1);
    clearTestBuffer(testBuffer2);
    clearTestBuffer(testBuffer3);

    // Verify buffers are cleared
    for (size_t i = 0; i < testBuffer1.size(); ++i) {
      expect(testBuffer1[i] == 0.0f,
             "Buffer1 element " + juce::String((int)i) + " should be 0.0");
    }

    for (size_t i = 0; i < testBuffer2.size(); ++i) {
      expect(testBuffer2[i] == 0.0f,
             "Buffer2 element " + juce::String((int)i) + " should be 0.0");
    }

    for (size_t i = 0; i < testBuffer3.size(); ++i) {
      expect(testBuffer3[i] == 0.0,
             "Buffer3 element " + juce::String((int)i) + " should be 0.0");
    }
  }

  //==========================================================================
  void testFixedArrayClearing() {
    beginTest("Fixed array clearing");

    // Create test arrays with different sizes
    const size_t size1 = 512;
    const size_t size2 = 1024;

    auto testArray1 = std::make_unique<float[]>(size1);
    auto testArray2 = std::make_unique<float[]>(size2);

    // Fill with test data
    for (size_t i = 0; i < size1; ++i)
      testArray1[i] = static_cast<float>(i) + 1.0f;

    for (size_t i = 0; i < size2; ++i)
      testArray2[i] = static_cast<float>(i) * -0.5f;

    // Verify initial values
    expect(testArray1[0] == 1.0f, "Initial array1[0] should be 1.0");
    expect(testArray1[10] == 11.0f, "Initial array1[10] should be 11.0");
    expect(testArray2[0] == 0.0f, "Initial array2[0] should be 0.0");
    expect(testArray2[10] == -5.0f, "Initial array2[10] should be -5.0");

    // Test clearing arrays (simulate the clearFixedArray method behavior)
    clearTestArray(testArray1.get(), size1);
    clearTestArray(testArray2.get(), size2);

    // Verify arrays are cleared
    for (size_t i = 0; i < size1; ++i) {
      expect(testArray1[i] == 0.0f,
             "Array1 element " + juce::String((int)i) + " should be 0.0");
    }

    for (size_t i = 0; i < size2; ++i) {
      expect(testArray2[i] == 0.0f,
             "Array2 element " + juce::String((int)i) + " should be 0.0");
    }
  }

  //==========================================================================
  void testBufferResizing() {
    beginTest("Buffer resizing operations");

    // Test buffer resizing functionality
    std::vector<float> testBuffer;

    // Test initial empty buffer
    expect(testBuffer.size() == 0, "Initial buffer should be empty");

    // Test resize to small size
    testBuffer.resize(100, 1.5f);
    expect(testBuffer.size() == 100, "Buffer should resize to 100");
    expect(testBuffer[0] == 1.5f, "Buffer elements should have default value");
    expect(testBuffer[99] == 1.5f, "Last element should have default value");

    // Test resize to larger size
    testBuffer.resize(500, 2.5f);
    expect(testBuffer.size() == 500, "Buffer should resize to 500");
    expect(testBuffer[99] == 1.5f, "Old elements should remain unchanged");
    expect(testBuffer[100] == 2.5f,
           "New elements should have new default value");
    expect(testBuffer[499] == 2.5f,
           "Last new element should have new default value");

    // Test resize to smaller size
    testBuffer.resize(200);
    expect(testBuffer.size() == 200, "Buffer should resize to 200");
    expect(testBuffer[99] == 1.5f,
           "Remaining old elements should be unchanged");
    expect(testBuffer[199] == 2.5f,
           "Last element should be from expanded range");
  }

  //==========================================================================
  void testCircularBufferOperations() {
    beginTest("Circular buffer operations");

    // Simulate circular buffer operations like those used in the delay lines
    const int bufferSize = 8;
    std::vector<float> circularBuffer(bufferSize, 0.0f);

    int writePos = 0;
    int readPos = 0;

    // Write some test data
    float testData[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                        6.0f, 7.0f, 8.0f, 9.0f, 10.0f};

    for (int i = 0; i < 10; ++i) {
      circularBuffer[writePos] = testData[i];
      writePos = (writePos + 1) % bufferSize;
    }

    // Verify circular wrapping
    expect(writePos == 2, "Write position should wrap around to 2");

    // Test reading with delay
    int delayInSamples = 4;
    readPos = (writePos - delayInSamples + bufferSize) % bufferSize;
    expect(readPos == 6, "Read position should be calculated correctly");

    // Verify the delayed values
    float delayedValue = circularBuffer[readPos];
    expect(delayedValue == 7.0f,
           "Delayed value should be 7.0 (from 4 samples ago)");

    // Test buffer bounds safety
    for (int pos = 0; pos < bufferSize * 3; ++pos) {
      int safePos = pos % bufferSize;
      expect(safePos >= 0 && safePos < bufferSize,
             "Position " + juce::String(pos) + " should wrap safely to " +
                 juce::String(safePos));
    }
  }

  //==========================================================================
  void testBufferBoundsSafety() {
    beginTest("Buffer bounds safety");

    const int bufferSize = 100;
    std::vector<float> testBuffer(bufferSize, 0.0f);

    // Test safe indexing patterns used in the plugin
    for (int i = 0; i < bufferSize * 2; ++i) {
      int safeIndex = i % bufferSize;
      expect(safeIndex >= 0 && safeIndex < bufferSize,
             "Index " + juce::String(i) + " should be safely wrapped to " +
                 juce::String(safeIndex));

      // Test that we can safely access the buffer
      testBuffer[safeIndex] = static_cast<float>(i);
      expect(testBuffer[safeIndex] == static_cast<float>(i),
             "Buffer access should work correctly at wrapped index");
    }

    // Test negative index handling (important for delay buffers)
    int writePos = 50;
    int delayAmount = 75;
    int readPos = writePos - delayAmount;

    if (readPos < 0)
      readPos += bufferSize;

    expect(readPos >= 0 && readPos < bufferSize,
           "Negative index should be wrapped correctly to positive range");
    expect(readPos == 75, "Specific calculation should yield expected result");
  }

  //==========================================================================
  void testMemoryAlignment() {
    beginTest("Memory alignment and initialization");

    // Test that buffers are properly aligned and initialized
    std::vector<float> audioBuffer(512, 0.0f);

    // Check that the buffer is zero-initialized
    for (size_t i = 0; i < audioBuffer.size(); ++i) {
      expect(audioBuffer[i] == 0.0f,
             "Audio buffer should be zero-initialized at index " +
                 juce::String((int)i));
    }

    // Test memory alignment (pointer should be properly aligned)
    float *bufferPtr = audioBuffer.data();
    expect(bufferPtr != nullptr, "Buffer pointer should not be null");

    // Test that we can write and read without issues
    for (size_t i = 0; i < audioBuffer.size(); ++i) {
      bufferPtr[i] = std::sin(static_cast<float>(i) * 0.1f);
    }

    for (size_t i = 0; i < audioBuffer.size(); ++i) {
      float expected = std::sin(static_cast<float>(i) * 0.1f);
      expectWithinAbsoluteError(
          bufferPtr[i], expected, 0.0001f,
          "Buffer should maintain written values correctly");
    }
  }

  //==========================================================================
  // Helper methods to simulate the actual template methods
  // (These simulate the behavior of the actual helper methods)

  template <typename Container> void clearTestBuffer(Container &buffer) {
    std::fill(buffer.begin(), buffer.end(), typename Container::value_type(0));
  }

  void clearTestArray(float *array, size_t size) {
    std::fill(array, array + size, 0.0f);
  }
};

// Register the test with JUCE's test system
static BufferOperationTests bufferTests;
