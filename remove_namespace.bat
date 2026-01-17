@echo off
echo Removing namespace speckit::log from header files...
echo.

cd /d %~dp0

REM List of header files to process
set "headers=log_buffer.h tag_filter.h file_manager.h crash_handler.h async_queue.h async_logger.h logger.h features.h"

for %%h in (%headers%) do (
    echo Processing: src\include\speckit\log\%%h
    powershell -Command "$content = Get-Content 'src\include\speckit\log\%%h' -Raw; $content = $content -replace 'namespace speckit::log \{', ''; $content = $content -replace '\} \/\/ namespace speckit::log', ''; Set-Content 'src\include\speckit\log\%%h' $content -NoNewline"
    if errorlevel 1 (
        echo   ERROR: Failed to process %%h
    ) else (
        echo   Done
    )
)

echo.
echo Processing source files...
echo.

REM List of source files to process
set "sources=log_entry.cpp log_buffer.cpp tag_filter.cpp file_manager.cpp crash_handler.cpp async_queue.cpp async_logger.cpp logger.cpp"

for %%s in (%sources%) do (
    echo Processing: src\src\%%s
    powershell -Command "$content = Get-Content 'src\src\%%s' -Raw; $content = $content -replace 'namespace speckit::log \{', ''; $content = $content -replace '\} \/\/ namespace speckit::log', ''; Set-Content 'src\src\%%s' $content -NoNewline"
    if errorlevel 1 (
        echo   ERROR: Failed to process %%s
    ) else (
        echo   Done
    )
)

echo.
echo ================================================
echo Namespace removal complete!
echo ================================================
echo.
echo Note: You may need to manually fix some code
echo if there are nested namespace issues.
echo.
pause
