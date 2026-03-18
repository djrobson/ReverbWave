/*
  ==============================================================================

    TestMain.cpp
    Created: 2025
    Author:  Claude Code

  ==============================================================================

  Main entry point for ReverbWave unit tests using JUCE UnitTest framework.

  This file initializes the JUCE test runner and executes all registered tests.
  Tests are automatically discovered through the UnitTest class registration
  system.

  Usage:
    ReverbWaveTests                    # Run all tests
    ReverbWaveTests --category DSP     # Run specific category
    ReverbWaveTests --verbose          # Verbose output
*/

#include <JuceHeader.h>

// Forward declarations for test classes
class ParameterManagementTests;
class BufferOperationTests;
class DSPAlgorithmTests;

//==============================================================================
/**
 * Custom console logger for test output formatting
 */
class TestConsoleLogger : public juce::Logger {
public:
  TestConsoleLogger() = default;

  void logMessage(const juce::String &message) override {
    std::cout << "[TEST] " << message << std::endl;
  }
};

//==============================================================================
/**
 * Main test runner application
 */
class TestRunner {
public:
  static int runAllTests() {
    // Set up custom logger
    auto logger = std::make_unique<TestConsoleLogger>();
    juce::Logger::setCurrentLogger(logger.get());

    // Initialize JUCE
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    // Get the UnitTest runner
    juce::UnitTestRunner testRunner;

    // Run all tests
    std::cout << "=== ReverbWave Unit Tests ===" << std::endl;
    std::cout << "Starting test execution..." << std::endl;

    testRunner.runAllTests();

    // Report results
    int numTestsPassed = 0;
    int numTestsFailed = 0;

    for (auto *test : juce::UnitTest::getAllTests()) {
      auto results = test->getResults();
      numTestsPassed += results.passed;
      numTestsFailed += results.failed;

      if (results.failed > 0) {
        std::cout << "FAILED: " << test->getName() << " (" << results.failed
                  << " failures)" << std::endl;
      }
    }

    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Passed: " << numTestsPassed << std::endl;
    std::cout << "Failed: " << numTestsFailed << std::endl;
    std::cout << "Total:  " << (numTestsPassed + numTestsFailed) << std::endl;

    // Clean up logger
    juce::Logger::setCurrentLogger(nullptr);

    return numTestsFailed == 0 ? 0 : 1;
  }

  static int runTestCategory(const juce::String &category) {
    auto logger = std::make_unique<TestConsoleLogger>();
    juce::Logger::setCurrentLogger(logger.get());

    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    juce::UnitTestRunner testRunner;

    std::cout << "Running tests in category: " << category << std::endl;

    // Run tests that match the category
    bool foundTests = false;
    for (auto *test : juce::UnitTest::getAllTests()) {
      if (test->getName().containsIgnoreCase(category)) {
        foundTests = true;
        testRunner.runTest(test);
      }
    }

    if (!foundTests) {
      std::cout << "No tests found in category: " << category << std::endl;
      return 1;
    }

    juce::Logger::setCurrentLogger(nullptr);
    return 0;
  }
};

//==============================================================================
/**
 * Main entry point for the test executable
 */
int main(int argc, char *argv[]) {
  juce::ArgumentList args(argc, argv);

  if (args.containsOption("--help") || args.containsOption("-h")) {
    std::cout << "ReverbWave Test Runner" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  ReverbWaveTests                  Run all tests"
              << std::endl;
    std::cout << "  ReverbWaveTests --category DSP   Run specific category"
              << std::endl;
    std::cout << "  ReverbWaveTests --help           Show this help"
              << std::endl;
    return 0;
  }

  if (args.containsOption("--category")) {
    auto category = args.getValueForOption("--category");
    if (category.isNotEmpty()) {
      return TestRunner::runTestCategory(category);
    }
  }

  // Run all tests by default
  return TestRunner::runAllTests();
}
