@echo off
setlocal enabledelayedexpansion

echo WAV to AHM File Renamer
echo ----------------------

set /a count=0
set /a errors=0

:: Check if any WAV files exist
dir /b *.wav >nul 2>&1
if errorlevel 1 (
    echo No WAV files found in the current directory.
    goto :end
)

:: List all WAV files before conversion
echo Found WAV files:
for %%F in (*.wav) do (
    echo   %%F
)
echo.

:: Prompt for confirmation
set /p confirm="Do you want to rename these files to .AHM? (Y/N): "
if /i "%confirm%" neq "Y" goto :end

:: Perform the renaming
for %%F in (*.wav) do (
    set "oldname=%%F"
    set "newname=%%~nF.AHM"
    
    ren "!oldname!" "!newname!" >nul 2>&1
    if errorlevel 1 (
        echo Error renaming !oldname!
        set /a errors+=1
    ) else (
        echo Renamed: !oldname! to !newname!
        set /a count+=1
    )
)

echo.
echo Conversion complete.
echo Successfully renamed files: %count%
if %errors% gtr 0 echo Failed to rename files: %errors%

:end
echo.
pause
