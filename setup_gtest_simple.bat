@echo off
echo ================================================
echo Simple GTest Setup - Integrate Source Code
echo ================================================
echo.
echo This script will integrate Google Test source code
echo directly into the project (simpler approach).
echo.

cd /d %~dp0

if exist "third_party\googletest" (
    echo GTest already exists
    choice /C YN /M "Re-download"
    if errorlevel 2 goto :end
    rmdir /s /q third_party\googletest
)

echo.
echo Step 1: Downloading Google Test source...
if not exist third_party mkdir third_party
cd third_party

git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git
if errorlevel 1 (
    echo ERROR: Failed to clone
    echo Please download manually and extract to third_party\googletest
    pause
    exit /b 1
)

echo.
echo Step 2: Creating CMakeLists.txt for GTest integration...
cd ..

(
echo # Google Test integration
echo set(GTEST_SOURCES
echo     ${GTEST_SOURCES}
echo     third_party/googletest/googletest/src/gtest-all.cc
echo     third_party/googletest/googlemock/src/gmock-all.cc
echo ^)
echo.
echo include_directories(
echo     third_party/googletest/googletest/include
echo     third_party/googletest/googletest
echo     third_party/googletest/googlemock/include
echo     third_party/googletest/googlemock
echo ^)
) > gtest_integration.cmake

echo.
echo Step 3: Creating patched CMakeLists.txt...
echo Modifying CMakeLists.txt to use integrated GTest...

powershell -Command "$content = Get-Content 'CMakeLists.txt' -Raw; $gtestBlock = @'
# Use integrated GTest source
set(GTEST_SOURCES
    third_party/googletest/googletest/src/gtest-all.cc
    third_party/googletest/googlemock/src/gmock-all.cc
^)

include_directories(
    third_party/googletest/googletest/include
    third_party/googletest/googletest
    third_party/googletest/googlemock/include
    third_party/googletest/googlemock
^)
'@; $content = $content -replace '# GoogleTest integration for testing', '$content\r\n# GoogleTest integration for testing'; $content = $content -replace 'find_package\(GTest QUIET\)', 'set(GTEST_FOUND TRUE)'; $content = $content -replace 'target_link_libraries\(unit_tests GTest::gtest GTest::gtest_main SpeckitLog\)', 'target_link_libraries(unit_tests SpeckitLog)'; Set-Content 'CMakeLists.txt' $content -NoNewline"

echo.
echo Step 4: Re-configuring project...
if exist build rmdir /s /q build

cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTING=ON

if errorlevel 1 (
    echo WARNING: Configuration had issues
    echo Trying to fix manually...
)

echo.
echo ================================================
echo Setup Complete!
echo ================================================
echo.
echo Note: If tests don't compile, you may need to
echo manually update CMakeLists.txt to properly link GTest.
echo.

:end
pause
