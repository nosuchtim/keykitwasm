@echo off
REM Simpler version - just call the bash script via Git Bash if available
REM This is more reliable than trying to replicate bash logic in batch

cd /d "%~dp0"

REM Try to find bash
where bash >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Found bash, using generate_manifest.sh...
    bash generate_manifest.sh
    exit /b 0
)

REM Try Git Bash in common locations
if exist "C:\Program Files\Git\bin\bash.exe" (
    echo Found Git Bash, using generate_manifest.sh...
    "C:\Program Files\Git\bin\bash.exe" generate_manifest.sh
    exit /b 0
)

if exist "C:\Program Files (x86)\Git\bin\bash.exe" (
    echo Found Git Bash, using generate_manifest.sh...
    "C:\Program Files (x86)\Git\bin\bash.exe" generate_manifest.sh
    exit /b 0
)

echo ERROR: bash not found. Please install Git for Windows or use WSL.
echo You can also run this in Git Bash: bash generate_manifest.sh
exit /b 1
