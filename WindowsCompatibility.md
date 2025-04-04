# Windows Compatibility Guide for SimpleReverb

To make SimpleReverb fully compatible with Windows, the following changes are needed:

## 1. Include Windows-specific Headers

Add these headers at the top of the file (after the existing includes):

```cpp
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

## 2. Terminal Initialization for Color Support

Add this function before the main function:

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

Then call this function at the start of main():

```cpp
int main() {
    // Initialize terminal for cross-platform color support
    initTerminal();
    
    // Rest of the main function...
}
```

## 3. Non-blocking Input Handling

Replace the platform-dependent input handling code:

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

## 4. CMakeLists.txt Updates

Update the target_link_libraries section to be platform-specific:

```cmake
# Platform-specific libraries
if(UNIX)
    target_link_libraries(SimpleReverb PRIVATE m pthread)
elseif(WIN32)
    target_link_libraries(SimpleReverb PRIVATE winmm)
endif()
```

These changes will make SimpleReverb fully compatible with both Windows and Unix-like systems.
