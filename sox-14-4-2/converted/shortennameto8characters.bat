@echo off
setlocal enabledelayedexpansion

rem Loop through all files in the current directory
for %%f in (*) do (
    
    rem Get the filename without extension
    set "filename=%%~nf"
    rem Get the file extension
    set "extension=%%~xf"

    rem Remove the first 3 characters from the filename
    set "newname=!filename:~3!"

    rem Truncate the filename to 8 characters if necessary
    set "newname=!newname:~0,8!"

    rem Initialize a counter for appending numbers if file already exists
    set "counter=1"
    
    rem Store the original newname before modifying for duplicates
    set "original_newname=!newname!"

    rem Check if the file with the new name already exists
    :check_exists
    if exist "!newname!!extension!" (
        rem Replace the last character with the counter value
        set "newname=!original_newname:~0,7!!counter!"
        set /a counter+=1
        goto check_exists
    )

    rem Rename the file to the new truncated name while preserving the extension
    ren "%%f" "!newname!!extension!"
)

echo Done renaming files.
pause
