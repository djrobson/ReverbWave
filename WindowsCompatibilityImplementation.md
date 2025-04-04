# Windows Compatibility Implementation Guide

This document provides step-by-step instructions for implementing Windows compatibility changes in the SimpleReverb application.

## Step 1: Update the Includes

At the top of `SimpleReverb.cpp`, after the existing includes, add these platform-specific includes:

```cpp
// Add after #include "SpectrumAnalyzer.h"

// Windows specific includes
#ifdef _WIN32
    #include <conio.h>    // For _kbhit() and _getch()
    #include <windows.h>  // For Windows specific functionality
#endif

// Define M_PI for Windows if it's not defined
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// For Unix-like systems
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif
```

## Step 2: Add Terminal Initialization Function

Add this function near the `clearScreen()` function:

```cpp
void initTerminal() {
    #ifdef _WIN32
        // Enable virtual terminal processing for color support on Windows
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
        SetConsoleMode(hOut, dwMode);
    #endif
}
```

## Step 3: Call Terminal Initialization in Main Function

At the beginning of the `main()` function, add:

```cpp
int main() {
    // Initialize terminal for cross-platform color support
    initTerminal();
    
    // Rest of the existing main function code...
```

## Step 4: Update the Non-blocking Input Handling

Find the section with platform-dependent input handling (search for "Non-blocking input check") and replace it with:

```cpp
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
    // For Unix-like systems, implement non-blocking input with select
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
```

## Step 5: Update CMakeLists.txt

Find the section in `CMakeLists.txt` where SimpleReverb is linked to libraries:

```cmake
target_link_libraries(SimpleReverb PRIVATE m pthread)
```

And replace it with:

```cmake
# Platform-specific libraries
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()
```

## Step 6: Testing the Changes

After implementing all these changes:

1. Test on Linux by building with cmake:
   ```
   mkdir -p build && cd build && cmake .. && make
   ```

2. Run the SimpleReverb application:
   ```
   ./SimpleReverb
   ```

3. If building on Windows, use:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

## Additional Notes

1. The `clearScreen()` function already contains proper platform detection for Windows vs. Unix systems.

2. Non-Windows platforms now have interactive keyboard input handling similar to Windows, which improves the user experience on all platforms.

3. The Windows-specific terminal initialization enables proper ANSI color processing on modern Windows terminals, ensuring the visualizations display correctly.

4. The platform-specific linking ensures that the appropriate system libraries are used on each platform.
