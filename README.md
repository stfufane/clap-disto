# CLAP Distortion

A cross-platform audio effect built with [CLAP](github.com/free-audio/clap) using the [visage](https://github.com/VitalAudio/visage) library for the user interface. 

## Build

It should build and run on Windows, macOS and Linux as long as you have a c++20 compiler and CMake installed.

```
# Generate
cmake . -B build
# Build
cmake --build build # --config Release
```

To test it, just scan the build folder with a clap-compatible DAW (tested with Bitwig and Reaper) and use it on an audio track to make it sound like shit.