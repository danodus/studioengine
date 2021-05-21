# Studio Engine

Engine for the development of digital audio applications using Lua.

## Getting Started

- Requires an installation of SDL2. For Linux:
```bash
sudo apt install libsdl2-dev
```
For Windows, set the SDL2_ROOT environment variable.
- Optional: Copy a General MIDI soundfont to "Resources/GM.sf2"
- Build the project with your IDE or manually:
```bash
cmake -B build -S .
cmake --build build
```
- Start the engine:
```bash
cd build
./studioengine
```
Press F10 to load/reload the script. The entry point is "Resources/main.lua".

To debug the Lua code, install the Lua Remote Debugger (LRDB) extension on Visual Studio Code. Press F9 to start the application in debug mode and attach the debugger to the process.
