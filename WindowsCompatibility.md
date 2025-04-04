# Windows Compatibility Guide for SimpleReverb

This guide describes the Windows compatibility features that have been implemented in SimpleReverb.

## Current Implementation Status

All Windows compatibility features have been successfully implemented and tested. The SimpleReverb application now runs natively on both Windows and Unix-like systems with identical functionality.

## Implemented Windows Compatibility Features

### 1. Windows-specific Headers

The following headers are included for Windows compatibility:

```cpp
// Windows specific includes
#ifdef _WIN32
#define NOMINMAX
    #include <conio.h>    // For _kbhit() and _getch()
    #include <windows.h>  // For Windows specific functionality
#else
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <fcntl.h>
#endif
```

### 2. Terminal Initialization for Color Support

A cross-platform terminal initialization function is implemented:

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

This function is called at the start of the main function:

```cpp
int main() {
    // Initialize terminal for cross-platform color support
    initTerminal();
    // ...
}
```

### 3. Cross-Platform Screen Clearing

```cpp
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
```

### 4. Non-blocking Input Handling

Platform-specific non-blocking input handling:

```cpp
// Non-blocking input check (platform dependent)
#ifdef _WIN32
    if (_kbhit()) {
        char key = tolower(_getch());
        // Process key presses
    }
#else
    // Unix implementation with select()
    struct timeval tv;
    fd_set fds;
    // Set up and check for input
#endif
```

### 5. Platform-Specific Library Linking

The CMakeLists.txt file includes platform-specific library linking:

```cmake
# Platform-specific libraries
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()
```

## Building on Windows

To build SimpleReverb on Windows:

1. Ensure you have CMake and a C++ compiler (Visual Studio or MinGW) installed
2. Open a command prompt in the project directory
3. Run the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. Run the application:
   ```
   Release\SimpleReverb.exe
   ```

## Testing

The application has been tested on:
- Windows 10/11 with Visual Studio 2019/2022
- Windows 10/11 with MinGW
- Ubuntu Linux (20.04 and later)

All platform-specific features work correctly on both platforms, including:
- Terminal color support
- Keyboard input handling
- Audio file generation with multiple frequencies (200, 400, 800, 1600, 3200, and 6400 Hz)
- Spectrum visualization in all three animation modes

## Note on JUCE Integration

The JUCE integration has been removed from the project for simplification. The application now runs as a standalone C++ application without JUCE dependencies, which improves cross-platform compatibility and build times.
