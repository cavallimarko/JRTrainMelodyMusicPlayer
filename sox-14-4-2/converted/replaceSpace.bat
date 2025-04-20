@echo off
setlocal enabledelayedexpansion

:: Set the directory to the one containing the files
set "directory=C:\path\to\your\directory"

:: Change to the specified directory
cd /d "%directory%"

:: Loop through all files in the directory
for %%f in (* *) do (
    set "oldname=%%f"
    set "newname=!oldname: =_!"

    :: Only rename if the new name is different
    if not "!oldname!"=="!newname!" (
        ren "!oldname!" "!newname!"
        echo Renamed "!oldname!" to "!newname!"
    )
)

endlocal
