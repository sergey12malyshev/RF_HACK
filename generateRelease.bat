@echo off
rem Automatic assembly of the release archive with the archive name set according to the latest GIT tag
chcp 1251 > nul

echo ========================================
echo    Generate Release Script
echo ========================================
echo.

:: Check if PowerShell is available
where powershell > nul 2>&1
if errorlevel 1 (
    echo [ERROR] PowerShell not found!
    pause
    exit /b 1
)

:: Optional: Check if Git is available (for informational purposes)
where git > nul 2>&1
if errorlevel 1 (
    echo [WARNING] Git not found in PATH
    echo.
)

echo [INFO] Starting release generation...
echo.

:: Execute PowerShell script with bypass execution policy
PowerShell.exe -NoProfile -ExecutionPolicy Bypass -File "scripts/generateRelease.ps1"

:: Check execution result
if errorlevel 1 (
    echo.
    echo [ERROR] Release generation failed!
    echo.
    pause
    exit /b 1
) else (
    echo.
    echo [SUCCESS] Release generated successfully!
    echo.
)

:: Optionally display contents of release folder if it exists
if exist "release" (
    echo Contents of release folder:
    dir /b "release" 2>nul
    echo.
)

pause
exit /b 0