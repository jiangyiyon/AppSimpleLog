@echo off
REM Unit Test Execution Script for AppSimpleLog P1 MVP

echo ========================================
echo Running Unit Tests - AppSimpleLog P1 MVP
echo ========================================

REM Navigate to build directory
cd /d "%~dp0"

REM Check if build directory exists
if not exist build (
    echo ERROR: build directory not found
    echo Please run vs_build.bat first to compile the project
    echo.
    pause
    exit /b 1
)

REM Check if Release directory exists
if not exist build\Release (
    echo ERROR: build\Release directory not found
    echo Please run vs_build.bat first to compile the project
    echo.
    pause
    exit /b 1
)

REM Check if unit_tests.exe exists
if not exist build\Release\unit_tests.exe (
    echo ERROR: unit_tests.exe not found
    echo Please run vs_build.bat first to compile the project
    echo.
    pause
    exit /b 1
)

cd build\Release

REM Run unit tests
echo Executing unit_tests.exe...
echo.
echo This will run all unit tests:
echo   - test_logger (Logger functionality)
echo   - test_log_entry (LogEntry formatting)
echo   - test_async_queue (AsyncQueue thread-safety)
echo   - test_async_logging (AsyncLogger operations)
echo   - test_process_isolation (PID detection)
echo   - test_file_manager (FileManager operations)
echo.
echo ========================================
echo.

unit_tests.exe

if errorlevel 1 (
    echo ========================================
    echo Unit tests failed with error code: %errorlevel%
    echo ========================================
    echo.
    echo Please check the output above for specific test failures
    echo.
) else (
    echo ========================================
    echo Unit tests completed successfully!
    echo ========================================
    echo.
    echo Check the test output above for pass/fail status
    echo.
    echo Expected: ~30 test cases across 6 test suites
    echo.
)

echo.
echo Press any key to exit...
pause >nul
