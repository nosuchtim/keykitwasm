@echo off
REM Generate a manifest of all library files for runtime loading
REM This batch file calls the Python script for reliable cross-platform operation

cd /d "%~dp0"

REM Try Python 3
python generate_manifest.py
if %ERRORLEVEL% EQU 0 goto :done

REM Try python3 command
python3 generate_manifest.py
if %ERRORLEVEL% EQU 0 goto :done

REM Try bash as fallback
where bash >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Python not found, using bash...
    bash generate_manifest.sh
    if %ERRORLEVEL% EQU 0 goto :done
)

:error
echo ERROR: Could not generate manifest
echo Please install Python 3 or Git Bash
exit /b 1

:done
exit /b 0
