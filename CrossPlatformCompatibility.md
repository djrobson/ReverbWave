# Cross-Platform Compatibility for ReverbVST

This document describes the changes implemented to make ReverbVST cross-platform compatible with Windows, in addition to its existing Linux/Unix compatibility.

## Overview of Platform-Specific Modifications

ReverbVST has been updated with the following platform-specific modifications:

1. Modified CMake configuration for platform-specific library linking

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
