@echo off
REM Build solution (Debug x64)
if not exist "SteelRevenant.sln" (
  echo Solution file not found in current directory
  exit /b 1
)n:: Use msbuild from environment (Visual Studio Developer Command Prompt)
msbuild "SteelRevenant.sln" /p:Configuration=Debug /p:Platform=x64 /mnif %errorlevel% neq 0 (
  echo Build failed
  exit /b %errorlevel%
)necho Build succeeded
