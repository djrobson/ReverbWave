@echo off
echo Building SimpleReverb for Windows...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Navigate to build directory
cd build

REM Run CMake to generate build files
echo Running CMake...
cmake ..

REM Build the project
echo Building SimpleReverb...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo Build failed with error %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

echo.
echo Build successful!
echo.
echo To run SimpleReverb, use: SimpleReverb.exe input.wav output.wav
echo.
echo Press any key to exit...
pause > nul
exit /b 0
