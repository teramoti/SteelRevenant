# SteelRevenant

SteelRevenant is a small DirectXTK-based action prototype focusing on primitive melee gameplay.

Requirements
- Windows, Visual Studio 2019 or 2022
- DirectXTK and DirectX11 SDK (as used by the project)

Quick build (Windows)
1. Open `SteelRevenant.sln` in Visual Studio.
2. Select `Debug | x64` configuration and build the solution.

Packaging
- A `build.bat` is provided for command-line builds (Visual Studio Developer Command Prompt).
- A `package.ps1` PowerShell script packages build artifacts. Example:
  `powershell -ExecutionPolicy Bypass -File package.ps1 -Configuration Debug -Platform x64`

Controls
- Move: WASD
- Aim: Mouse
- Attack: Left click
- Guard: Right click
- Slide: Hold Shift
- Toggle UI: Tab
- Back: Esc

Notes
- Source files should be saved as UTF-8.
- Audio resources are expected under `Resources/Sound`.
- If audio does not play, confirm that the WAV files exist in `Resources/Sound` and that the AudioSystem was initialized with the correct root path.

Development
- The project contains a simple combat system, enemy behaviors, and stage rule definitions.
- Title screen has a simple attract demo mode that starts after a period of inactivity.

See `BUILD_NOTES.md` for more detailed build information.
