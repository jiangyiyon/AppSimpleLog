# Unit Test Execution Guide

**Date**: 2025-01-16
**Purpose**: Execute unit tests for AppSimpleLog P1 MVP

---

## Prerequisites

Before running unit tests, ensure:

1. **Project is Built**
   - `build/Release\unit_tests.exe` exists
   - If not, run `vs_build.bat` first

2. **Build Artifacts Verified**
   - `SpeckitLog.lib` exists in `build\Release\`
   - `SpeckitBridge.dll` exists in `build\Release\`

---

## Execution Methods

### Option 1: Run Test Script (Recommended)

Execute the automated script:
```batch
cd e:\APPLogByAI\AppSimpleLog
run_unit_tests.bat
```

**What it does**:
- Validates build directory exists
- Checks for unit_tests.exe
- Runs all unit tests
- Provides clear pass/fail indication
- Pauses for review

---

### Option 2: Direct Execution

```batch
cd e:\APPLogByAI\AppSimpleLog\build\Release
unit_tests.exe
```

---

## Expected Test Output

When unit_tests.exe runs, you'll see:

```
[==========] Running 6 tests from 6 test cases.
[----------] Global test environment set-up.
[----------] 6 tests from LoggerTest
[ RUN      ] LoggerTest.SetAndGetLogLevel
[       OK ] LoggerTest.SetAndGetLogLevel
[ RUN      ] LoggerTest.ShouldLog_RespectsLevelFiltering
[       OK ] LoggerTest.ShouldLog_RespectsLevelFiltering
...
[----------] 6 tests from LoggerTest ran. (XYZ ms total)
[  PASSED  ] 6 tests.
[==========] 6 tests from 6 test cases ran. (XYZ ms total)
[==========] 100% tests passed.
```

**Expected Results**:
- **Test Suites**: 6 test suites
- **Total Test Cases**: ~30 tests
- **Expected Pass Rate**: 100% (all tests should pass)

---

## Test Suites

### 1. test_logger
- **Purpose**: Logger class functionality
- **Tests**: Level filtering, log formatting, basic operations
- **Expected**: All tests pass

### 2. test_log_entry
- **Purpose**: LogEntry struct functionality
- **Tests**: Constructor, move semantics, string views, formatting
- **Expected**: All tests pass

### 3. test_async_queue
- **Purpose**: AsyncQueue thread-safety
- **Tests**: Empty queue, push/pop operations, wrap-around, concurrency
- **Expected**: All tests pass

### 4. test_async_logging
- **Purpose**: AsyncLogger operations
- **Tests**: Async logging, level filtering, non-blocking submission
- **Expected**: All tests pass

### 5. test_process_isolation
- **Purpose**: Process ID detection
- **Tests**: PID not zero, PID consistency, thread ID capture
- **Expected**: All tests pass

### 6. test_file_manager
- **Purpose**: FileManager operations
- **Tests**: File name generation, first vs subsequent process
- **Expected**: All tests pass

---

## Troubleshooting

### Issue: unit_tests.exe Not Found

**Solution**:
- Ensure project is built: Run `vs_build.bat` first
- Check that `build\Release\unit_tests.exe` exists
- Verify build completed successfully

### Issue: DLL Missing

**Solution**:
- `SpeckitBridge.dll` must be in `build\Release\`
- If missing, rebuild project
- Ensure SpeckitLog.lib is available

### Issue: Test Failures

**Solution**:
- Check output for specific test failures
- Review error messages
- Fix source code issues if necessary
- Rebuild and re-run tests

### Issue: Linker Errors

**Solution**:
- Ensure GoogleTest is properly linked
- Check library paths
- Rebuild from Visual Studio if needed

---

## Validation Checklist

After running tests, verify:

- [ ] All 6 test suites executed
- [ ] Test output shows pass/fail status
- [ ] Error code is 0 (success)
- [ ] Expected test count (~30) matches actual
- [ ] No critical errors or crashes

---

## Next Steps

### After Unit Tests Pass

1. **Run Integration Tests**
   ```batch
   cd build\Release
   integration_tests.exe
   ```

2. **Run Performance Tests**
   ```batch
   cd build\Release
   performance_tests.exe
   ```

3. **Validate P1 MVP**
   - Verify all three user stories work correctly
   - Confirm <1ms latency requirement
   - Confirm 100K log throughput requirement

---

## Notes

The unit tests will validate:
- ✅ Logger level filtering
- ✅ LogEntry formatting and move semantics
- ✅ AsyncQueue thread-safety and operations
- ✅ AsyncLogger async operations
- ✅ Process ID detection and consistency
- ✅ File name generation logic

Execute `run_unit_tests.bat` to run all unit tests and verify P1 MVP functionality.
