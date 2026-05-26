@echo off
setlocal

cd /d "%~dp0"

call :find_python
if errorlevel 1 exit /b 1

echo Using Python: %PYTHON_CMD%
%PYTHON_CMD% create_dist.py
exit /b %errorlevel%

:find_python
set "PYTHON_CMD="

where py >nul 2>nul
if not errorlevel 1 (
	set "PYTHON_CMD=py -3"
	goto :found_python
)

where python >nul 2>nul
if not errorlevel 1 (
	set "PYTHON_CMD=python"
	goto :found_python
)

where python3 >nul 2>nul
if not errorlevel 1 (
	set "PYTHON_CMD=python3"
	goto :found_python
)

echo Unable to find Python. Install Python 3 or add it to PATH.
exit /b 1

:found_python
exit /b 0
