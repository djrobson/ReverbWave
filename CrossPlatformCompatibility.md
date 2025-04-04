# Cross-Platform Compatibility for SimpleReverb

This document describes the changes implemented to make SimpleReverb compatible with Windows, in addition to its existing Linux/Unix compatibility.

## Overview of Platform-Specific Modifications

SimpleReverb has been updated with the following platform-specific modifications:

1. Added Windows-specific includes for non-blocking input and terminal handling
2. Implemented cross-platform terminal color support
3. Enhanced keyboard input handling for both Windows and Unix-like systems
4. Modified CMake configuration for platform-specific library linking
5. Added terminal initialization for color support on Windows

## Detailed Description of Changes

### 1. Platform-Specific Headers

Windows requires different headers for non-blocking keyboard input and console control:

```cpp
// Windows specific includes
#ifdef _WIN32
    #include <conio.h>    // For _kbhit() and _getch()
    #include <windows.h>  // For Windows specific functionality
#endif

// For Unix-like systems
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif
```

### 2. Terminal Initialization for Color Support

Windows 10 and later consoles support ANSI colors, but require explicit initialization:

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

### 3. Cross-Platform Screen Clearing

The screen clearing function was already implemented for cross-platform support:

```cpp
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
```

### 4. Non-Blocking Keyboard Input Handling

Windows and Unix handle non-blocking input differently:

```cpp
// Windows version uses _kbhit() and _getch()
#ifdef _WIN32
    if (_kbhit()) {
        char key = tolower(_getch());
        // ... process key ...
    }
#else
    // Unix version uses select() for non-blocking input
    struct timeval tv;
    fd_set fds;
    // ... set up select() ...
    if (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0) {
        // ... read and process key ...
    }
#endif
```

### 5. Platform-Specific Library Linking

Different platforms require different system libraries:

```cmake
# Platform-specific libraries
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()
```

## Implementation Details

The implementation includes:

1. Proper platform detection using preprocessor defines (`#ifdef _WIN32`)
2. Fallback behaviors for non-interactive modes
3. Enhanced keyboard input handling on all platforms
4. Terminal color support across operating systems

## Testing Procedures

To verify the cross-platform compatibility:

1. On Linux/Unix:
   ```
   mkdir -p build && cd build && cmake .. && make
   ./SimpleReverb original.wav output.wav
   ```

2. On Windows:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   SimpleReverb.exe original.wav output.wav
   ```

## Known Issues and Limitations

1. Terminal color support in Windows requires Windows 10 version 1511 or later
2. The spectrum analyzer visualization quality depends on terminal/console capabilities
3. The interactive mode requires keyboard input support, which may not be available in all environments

## Future Improvements

1. Consider using a cross-platform library like ncurses for more advanced terminal handling
2. Implement a proper GUI using JUCE once all dependencies are resolved
3. Enhance the Windows compatibility with DirectSound or WASAPI support for real-time audio processing

## Conclusion

With these modifications, SimpleReverb now works consistently across Windows and Unix-like platforms, providing the same functionality and user experience regardless of the operating system.
