# SteelRevenant
# SteelRevenant

DirectX 11 / DirectXTK を使用した3Dアクションゲームです。近接戦闘・ロックオン・ウェーブサバイバル形式のステージで構成されています。

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
- ロックオン切替: Tab
- Back: Esc

Notes
- Source files should be saved as UTF-8.
- Audio resources are expected under `Assets/Audio`.
- If audio does not play, confirm that the WAV files exist in `Assets/Audio` and that the AudioSystem was initialized with the correct root path.

Development
- The project contains a combat system, enemy behaviors, and stage rule definitions.
- Title screen has an attract demo mode that starts after a period of inactivity.
