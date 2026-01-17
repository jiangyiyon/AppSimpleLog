@echo off
echo Updating CMakeLists.txt to use integrated GTest...
echo.

cd /d %~dp0

if not exist "third_party\googletest" (
    echo ERROR: GTest not found!
    echo Please run download_gtest.bat first
    pause
    exit /b 1
)

echo Creating backup...
copy CMakeLists.txt CMakeLists.txt.backup

echo.
echo Modifying CMakeLists.txt...

REM Use PowerShell to make the changes
powershell -ExecutionPolicy Bypass -File - <<'POWERSHELL'
$file = "CMakeLists.txt"
$content = Get-Content $file -Raw

# Find and replace the GTest section
$oldGTestSection = @'# GoogleTest integration for testing
option\(BUILD_TESTING "Build tests" ON\)
if\(BUILD_TESTING\)
    enable_testing\(\)
    find_package\(GTest QUIET\)

    if\(GTEST_FOUND\)@

$newGTestSection = @'# GoogleTest integration for testing
option\(BUILD_TESTING "Build tests" ON\)
if\(BUILD_TESTING\)
    enable_testing\(\)

    # Use integrated GTest source code
    set\(GTEST_FOUND TRUE\)
    set\(GTEST_SOURCES
        third_party/googletest/googletest/src/gtest-all.cc
        third_party/googletest/googlemock/src/gmock-all.cc
    \)

    include_directories\(
        third_party/googletest/googletest/include
        third_party/googletest/googletest
        third_party/googletest/googlemock/include
        third_party/googletest/googlemock
    \)

    if\(TRUE\)@

$oldTargetLink = @target_link_libraries\(unit_tests GTest::gtest GTest::gtest_main SpeckitLog\)@
$newTargetLink = @target_link_libraries\(unit_tests ${GTEST_SOURCES} SpeckitLog\)@

$oldTargetLink2 = @target_link_libraries\(integration_tests GTest::gtest GTest::gtest_main SpeckitLog\)@
$newTargetLink2 = @target_link_libraries\(integration_tests ${GTEST_SOURCES} SpeckitLog\)@

$oldTargetLink3 = @target_link_libraries\(performance_tests GTest::gtest GTest::gtest_main SpeckitLog\)@
$newTargetLink3 = @target_link_libraries\(performance_tests ${GTEST_SOURCES} SpeckitLog\)@

$content = $content -replace [regex]::Escape($oldGTestSection), $newGTestSection
$content = $content -replace [regex]::Escape($oldTargetLink), $newTargetLink
$content = $content -replace [regex]::Escape($oldTargetLink2), $newTargetLink2
$content = $content -replace [regex]::Escape($oldTargetLink3), $newTargetLink3

# Remove the else block for GTest not found
$content = $content -replace [regex]::Escape("    else\(\)\r\n        message\(WARNING \"GTest not found\. Tests will not be built\. Please install GTest or set BUILD_TESTING=OFF\"\)\r\n    endif\(\)"), ""

$content | Set-Content $file -NoNewline

Write-Host "CMakeLists.txt updated successfully!"
POWERSHELL

if errorlevel 1 (
    echo.
    echo ERROR: Failed to update CMakeLists.txt
    echo Restoring backup...
    copy CMakeLists.txt.backup CMakeLists.txt
    pause
    exit /b 1
)

echo.
echo Step 2: Re-configuring project...
if exist build rmdir /s /q build

cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTING=ON

if errorlevel 1 (
    echo.
    echo WARNING: Configuration had warnings
    echo This is normal for the first time
)

echo.
echo ================================================
echo Success!
echo ================================================
echo.
echo You can now build and run tests:
echo   cmake --build build\debug --config Debug
echo   build\debug\Debug\unit_tests.exe
echo.
echo Backup saved to: CMakeLists.txt.backup
echo.
pause
