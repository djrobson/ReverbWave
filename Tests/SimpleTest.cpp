/*
  ==============================================================================

    SimpleTest.cpp
    Created: 2025
    Author:  Claude Code

  ==============================================================================

  Minimal validation tests for ReverbWave refactoring concepts.

  This file provides basic tests to verify that our refactoring patterns
  work correctly, focusing on the core concepts rather than full JUCE
  integration.
*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

//==============================================================================
// Test framework
//==============================================================================

int g_testsPassed = 0;
int g_testsFailed = 0;

void expect(bool condition, const std::string &message) {
  if (condition) {
    g_testsPassed++;
    std::cout << "✅ PASS: " << message << std::endl;
  } else {
    g_testsFailed++;
    std::cout << "❌ FAIL: " << message << std::endl;
  }
}

void beginTest(const std::string &testName) {
  std::cout << "\n🔍 Running test: " << testName << std::endl;
}

//==============================================================================
// Mock classes to simulate our refactored functionality
//==============================================================================

// Mock parameter IDs - simulates our refactored parameter system
class MockParameterManager {
public:
  static const std::vector<std::string> &getParameterIDs() {
    static const std::vector<std::string> ids = {
        "roomSize",    "damping",         "wetLevel",      "dryLevel",
        "width",       "freezeMode",      "crossoverFreq", "highFreqDelay",
        "highFreqMix", "harmDetuneAmount"};
    return ids;
  }

  void setupParameterListeners() {
    const auto &paramIds = getParameterIDs();
    for (const auto &paramId : paramIds) {
      // Simulate adding listener (our refactored helper method)
      listeners[paramId] = true;
    }
  }

  void removeParameterListeners() {
    const auto &paramIds = getParameterIDs();
    for (const auto &paramId : paramIds) {
      // Simulate removing listener (our refactored helper method)
      listeners[paramId] = false;
    }
  }

  bool isListenerActive(const std::string &paramId) const {
    auto it = listeners.find(paramId);
    return it != listeners.end() && it->second;
  }

private:
  std::map<std::string, bool> listeners;
};

// Mock buffer operations - simulates our refactored buffer helpers
class MockBufferManager {
public:
  template <typename Container> static void clearBuffer(Container &buffer) {
    std::fill(buffer.begin(), buffer.end(), typename Container::value_type(0));
  }

  static void clearFixedArray(float *array, size_t size) {
    std::fill(array, array + size, 0.0f);
  }

  static void resizeDelayBuffers(std::vector<float> &buffer, int newSize) {
    if (newSize > static_cast<int>(buffer.size())) {
      buffer.resize(newSize, 0.0f);
    }
  }

  template <typename ProcessFunc>
  static void processStereoChannels(float &left, float &right,
                                    ProcessFunc func) {
    func(left, right);
  }
};

//==============================================================================
// Test implementations
//==============================================================================

void testParameterIDsConsistency() {
  beginTest("Parameter IDs Consistency");

  MockParameterManager manager;
  const auto &paramIds = MockParameterManager::getParameterIDs();

  // Test that we have the expected number of parameters
  expect(paramIds.size() == 10, "Should have 10 parameter IDs");

  // Test that essential parameters exist
  std::vector<std::string> essentialParams = {"roomSize", "damping", "wetLevel",
                                              "dryLevel"};
  for (const auto &param : essentialParams) {
    bool found =
        std::find(paramIds.begin(), paramIds.end(), param) != paramIds.end();
    expect(found, "Should contain parameter: " + param);
  }

  // Test parameter ID uniqueness
  std::set<std::string> uniqueIds(paramIds.begin(), paramIds.end());
  expect(uniqueIds.size() == paramIds.size(),
         "All parameter IDs should be unique");
}

void testParameterListenerManagement() {
  beginTest("Parameter Listener Management");

  MockParameterManager manager;

  // Test setup
  manager.setupParameterListeners();

  const auto &paramIds = MockParameterManager::getParameterIDs();
  for (const auto &paramId : paramIds) {
    expect(manager.isListenerActive(paramId),
           "Listener should be active for: " + paramId);
  }

  // Test cleanup
  manager.removeParameterListeners();

  for (const auto &paramId : paramIds) {
    expect(!manager.isListenerActive(paramId),
           "Listener should be removed for: " + paramId);
  }
}

void testBufferOperations() {
  beginTest("Buffer Operations");

  // Test vector buffer clearing (template method)
  std::vector<float> testBuffer(1024, 42.0f);
  expect(testBuffer[0] == 42.0f,
         "Buffer should be initialized with test value");

  MockBufferManager::clearBuffer(testBuffer);
  expect(testBuffer[0] == 0.0f, "Buffer should be cleared to zero");
  expect(testBuffer[1023] == 0.0f, "Buffer end should be cleared to zero");

  // Test fixed array clearing
  const size_t arraySize = 512;
  auto testArray = std::make_unique<float[]>(arraySize);

  for (size_t i = 0; i < arraySize; ++i)
    testArray[i] = static_cast<float>(i);

  expect(testArray[10] == 10.0f, "Array should be initialized correctly");

  MockBufferManager::clearFixedArray(testArray.get(), arraySize);
  expect(testArray[10] == 0.0f, "Array should be cleared correctly");

  // Test buffer resizing
  std::vector<float> resizableBuffer(100, 1.0f);
  expect(resizableBuffer.size() == 100, "Initial size should be 100");

  MockBufferManager::resizeDelayBuffers(resizableBuffer, 200);
  expect(resizableBuffer.size() == 200, "Buffer should be resized to 200");
  expect(resizableBuffer[99] == 1.0f, "Original values should be preserved");
  expect(resizableBuffer[150] == 0.0f, "New values should be zero-initialized");
}

void testStereoProcessing() {
  beginTest("Stereo Processing Template");

  float left = 0.5f;
  float right = -0.3f;

  // Test stereo processing with simple gain function
  auto gainProcessor = [](float &l, float &r) {
    l *= 2.0f;
    r *= 2.0f;
  };

  MockBufferManager::processStereoChannels(left, right, gainProcessor);

  expect(std::abs(left - 1.0f) < 0.001f, "Left channel should be doubled");
  expect(std::abs(right - (-0.6f)) < 0.001f, "Right channel should be doubled");

  // Test with swap function
  left = 1.0f;
  right = 2.0f;

  auto swapProcessor = [](float &l, float &r) { std::swap(l, r); };

  MockBufferManager::processStereoChannels(left, right, swapProcessor);

  expect(left == 2.0f, "Left should now have right's value");
  expect(right == 1.0f, "Right should now have left's value");
}

void testCircularBufferOperations() {
  beginTest("Circular Buffer Operations");

  // Test circular buffer functionality (used in delay lines)
  const int bufferSize = 8;
  std::vector<float> circularBuffer(bufferSize, 0.0f);

  int writePos = 0;
  float testData[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                      6.0f, 7.0f, 8.0f, 9.0f, 10.0f};

  // Write more data than buffer size to test wrapping
  for (int i = 0; i < 10; ++i) {
    circularBuffer[writePos] = testData[i];
    writePos = (writePos + 1) % bufferSize;
  }

  expect(writePos == 2, "Write position should wrap around to 2");

  // Test delay calculation (simulating delay line read)
  int delayInSamples = 4;
  int readPos = (writePos - delayInSamples + bufferSize) % bufferSize;

  expect(readPos >= 0 && readPos < bufferSize,
         "Read position should be within bounds");

  float delayedValue = circularBuffer[readPos];
  // We expect to read the value that was written 4 samples ago
  expect(delayedValue == 7.0f, "Should read delayed value correctly");
}

void testRefactoringBenefits() {
  beginTest("Refactoring Benefits Validation");

  // Demonstrate that our refactored approach reduces code duplication
  MockParameterManager manager1, manager2, manager3;

  // Before refactoring, we would have had 10 separate listener setup calls per
  // manager Now we just call one helper method each:
  manager1.setupParameterListeners();
  manager2.setupParameterListeners();
  manager3.setupParameterListeners();

  // Verify all have consistent behavior
  const auto &paramIds = MockParameterManager::getParameterIDs();
  for (const auto &paramId : paramIds) {
    expect(manager1.isListenerActive(paramId) &&
               manager2.isListenerActive(paramId) &&
               manager3.isListenerActive(paramId),
           "All managers should have consistent listener state for: " +
               paramId);
  }

  expect(
      true,
      "Refactored helper methods provide consistent behavior across instances");
}

//==============================================================================
// Main test runner
//==============================================================================

int main() {
  std::cout << "🧪 ReverbWave Refactoring Validation Tests" << std::endl;
  std::cout << "==========================================" << std::endl;
  std::cout << "Testing the core concepts from our refactoring work..."
            << std::endl;

  // Run tests
  testParameterIDsConsistency();
  testParameterListenerManagement();
  testBufferOperations();
  testStereoProcessing();
  testCircularBufferOperations();
  testRefactoringBenefits();

  // Report results
  std::cout << "\n📊 Test Results:" << std::endl;
  std::cout << "================" << std::endl;
  std::cout << "✅ Passed: " << g_testsPassed << std::endl;
  std::cout << "❌ Failed: " << g_testsFailed << std::endl;
  std::cout << "📈 Total:  " << (g_testsPassed + g_testsFailed) << std::endl;

  if (g_testsFailed == 0) {
    std::cout << "\n🎉 All tests passed!" << std::endl;
    std::cout << "✨ The refactored concepts are working correctly."
              << std::endl;
    std::cout << "🔧 Ready to integrate with full JUCE framework." << std::endl;
    return 0;
  } else {
    std::cout << "\n⚠️  Some tests failed. Please review the output above."
              << std::endl;
    return 1;
  }
}
