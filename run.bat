@echo off
cd /d "%~dp0"
if exist "build\FlappyBird.exe" (
    cd build
    start "" FlappyBird.exe
) else (
    echo FlappyBird.exe not found in build directory. Please compile first.
    pause
)
