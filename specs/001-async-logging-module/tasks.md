# Tasks: 高性能异步跨平台日志模块

**Input**: Design documents from `/specs/001-async-logging-module/`
**Prerequisites**: plan.md, spec.md

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/`, `cbridge/` at repository root
- Paths shown below assume single project structure from plan.md

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [ ] T001 Create project directory structure per implementation plan in src/, cbridge/, tests/
- [ ] T002 Create CMakeLists.txt with C++20 and GoogleTest configuration in CMakeLists.txt
- [ ] T003 [P] Create README.md with project overview and build instructions in README.md
- [ ] T004 [P] Setup platform detection macros for Windows/macOS in src/include/speckit/log/platform.h
- [ ] T005 [P] Setup compile-time feature detection for C++20 features in src/include/speckit/log/features.h

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T006 Create LogLevel enum with DEBUG, INFO, WARNING, ERROR in src/include/speckit/log/log_level.h
- [ ] T007 [P] Define LogEntry struct with level, timestamp, processId, threadId, tag, message in src/include/speckit/log/log_entry.h
- [ ] T008 [P] Define SpeckitLogger opaque pointer struct in cbridge/speckit_logger.h
- [ ] T009 [P] Create RingBuffer class for crash-safe logging in src/include/speckit/log/log_buffer.h
- [ ] T010 Implement LogEntry formatting with std::format in src/src/log_entry.cpp
- [ ] T011 [P] Implement RingBuffer with atomic operations in src/src/log_buffer.cpp
- [ ] T012 Implement TagFilter class with enable/disable and level configuration in src/include/speckit/log/tag_filter.h
- [ ] T013 Implement TagFilter filtering logic in src/src/tag_filter.cpp
- [ ] T014 Create crash handler with signal registration (SIGSEGV, SIGABRT) in src/include/speckit/log/crash_handler.h
- [ ] T015 Implement crash handler emergency flush in src/src/crash_handler.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - 基础日志记录与级别控制 (Priority: P1) 🎯 MVP

**Goal**: Provide core logging functionality with four levels (DEBUG, INFO, WARNING, ERROR) and dynamic level control

**Independent Test**: Create logger instance, set different log levels, record logs, verify only logs meeting level criteria are output to file

- [ ] T016 Create Logger class with SetLogLevel method in src/include/speckit/log/logger.h
- [ ] T017 [P] Create FileManager class for basic file operations in src/include/speckit/log/file_manager.h
- [ ] T018 [P] Implement FileManager base file operations in src/src/file_manager.cpp
- [ ] T019 [P] Implement Logger log level filtering in src/src/logger.cpp
- [ ] T020 Implement Logger log method with timestamp and basic formatting in src/src/logger.cpp
- [ ] T021 Create speckit_logger_create factory function in cbridge/speckit_logger.cpp
- [ ] T022 Create speckit_logger_log function in cbridge/speckit_logger.cpp
- [ ] T023 Create speckit_logger_set_log_level function in cbridge/speckit_logger.cpp
- [ ] T024 Create speckit_logger_destroy function in cbridge/speckit_logger.cpp
- [ ] T025 [P] Create unit test for log level filtering in tests/unit/test_logger.cpp
- [ ] T026 [P] Create unit test for LogEntry formatting in tests/unit/test_log_entry.cpp

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - 跨平台多进程日志隔离 (Priority: P1)

**Goal**: Ensure each process has independent log files with unique naming (base name or base_name_PID)

**Independent Test**: Launch multiple application instances, verify each creates independent log files with correct process IDs, no cross-contamination

- [ ] T027 Implement process ID detection and caching in src/src/logger.cpp
- [ ] T028 [P] Implement file name generation logic (base name vs base_name_PID) in src/src/file_manager.cpp
- [ ] T029 [P] Create unit test for process ID detection in tests/unit/test_logger.cpp
- [ ] T030 [P] Create unit test for file name generation in tests/unit/test_file_manager.cpp
- [ ] T031 Create integration test for multi-process logging in tests/integration/test_multi_process.cpp

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - 异步高性能日志写入 (Priority: P1)

**Goal**: Implement non-blocking async logging with <1ms call latency using background thread and lock-free queue

**Independent Test**: High-frequency logging program measures call latency, verifies <1ms return, background thread handles actual writes

- [ ] T032 Create AsyncQueue lock-free queue class in src/include/speckit/log/async_queue.h
- [ ] T033 Implement AsyncQueue with std::atomic operations in src/src/async_queue.cpp
- [ ] T034 Create background writer thread with std::jthread in src/src/logger.cpp
- [ ] T035 [P] Implement thread-safe log submission to queue in src/src/logger.cpp
- [ ] T036 [P] Implement background thread write loop with std::condition_variable_any in src/src/logger.cpp
- [ ] T037 Implement flush on normal exit in src/src/logger.cpp
- [ ] T038 Implement periodic flush from RingBuffer in src/src/logger.cpp
- [ ] T039 [P] Create unit test for AsyncQueue in tests/unit/test_async_queue.cpp
- [ ] T040 [P] Create unit test for async logging in tests/unit/test_logger.cpp
- [ ] T041 Create performance test for log latency in tests/performance/test_latency.cpp
- [ ] T042 Create performance test for throughput in tests/performance/test_throughput.cpp

**Checkpoint**: All P1 user stories should now be independently functional

---

## Phase 6: User Story 4 - 日志文件滚动与保留 (Priority: P2)

**Goal**: Automatic file rotation based on size limit with configurable retention (default 3 historical files)

**Independent Test**: Set small size limit, write logs until rotation, verify new file creation, old file renaming, retention limit compliance

- [ ] T043 [P] Implement file size detection in src/src/file_manager.cpp
- [ ] T044 [P] Implement file rotation logic (close current, rename with sequence, create new) in src/src/file_manager.cpp
- [ ] T045 [P] Implement retention policy (delete oldest beyond limit) in src/src/file_manager.cpp
- [ ] T046 Create speckit_logger_set_max_file_size function in cbridge/speckit_logger.cpp
- [ ] T047 Create speckit_logger_set_retention_count function in cbridge/speckit_logger.cpp
- [ ] T048 [P] Create unit test for file rotation in tests/unit/test_file_manager.cpp
- [ ] T049 Create integration test for file rotation in tests/integration/test_file_rotation.cpp

**Checkpoint**: At this point, User Stories 1-4 should all work independently

---

## Phase 7: User Story 5 - 日志标签分类与过滤 (Priority: P2)

**Goal**: Tag-based log categorization with enable/disable and per-tag level configuration

**Independent Test**: Log with different tags, enable/disable specific tags, set per-tag levels, verify filtering behavior

- [ ] T050 Add tag parameter to Logger::log method in src/include/speckit/log/logger.h
- [ ] T051 [P] Integrate TagFilter into Logger::log in src/src/logger.cpp
- [ ] T052 Create speckit_logger_set_tag_enabled function in cbridge/speckit_logger.cpp
- [ ] T053 Create speckit_logger_set_tag_level function in cbridge/speckit_logger.cpp
- [ ] T054 [P] Create unit test for tag filtering in tests/unit/test_tag_filter.cpp
- [ ] T055 Create unit test for tag filtering in Logger in tests/unit/test_logger.cpp

**Checkpoint**: At this point, all P1 and P2 user stories should work independently

---

## Phase 8: User Story 6 - 日志归档与压缩 (Priority: P3)

**Goal**: ZIP archiving of current and historical log files with configurable triggers (manual, time, size, exit)

**Independent Test**: Trigger archive, verify ZIP creation with correct naming, original file deletion, new file creation

- [ ] T056 [P] Create ZIP archiving interface in src/include/speckit/log/archive.h
- [ ] T057 Implement ZIP creation with process ID and timestamp in src/src/archive.cpp
- [ ] T058 [P] Implement file deletion after archive in src/src/archive.cpp
- [ ] T059 Create speckit_logger_archive function in cbridge/speckit_logger.cpp
- [ ] T060 Create speckit_logger_set_auto_archive_config function in cbridge/speckit_logger.cpp
- [ ] T061 [P] Create integration test for archiving in tests/integration/test_archive.cpp

**Checkpoint**: All user stories should now be independently functional

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T062 [P] Add NS_ENUM/CF_ENUM macros for C enums in cbridge/speckit_logger.h
- [ ] T063 [P] Implement disk space error handling in src/src/file_manager.cpp
- [ ] T064 [P] Implement file permission error handling in src/src/file_manager.cpp
- [ ] T065 Implement graceful degradation on buffer overflow in src/src/logger.cpp
- [ ] T066 [P] Create stress test for concurrent writes in tests/performance/test_stress.cpp
- [ ] T067 Create crash recovery test in tests/integration/test_crash_recovery.cpp
- [ ] T068 [P] Update README.md with full API documentation and usage examples in README.md
- [ ] T069 [P] Create quickstart.md with code examples in specs/001-async-logging-module/quickstart.md
- [ ] T070 [P] Run 24-hour stability test and document results in tests/performance/

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-8)**: All depend on Foundational phase completion
  - P1 stories (US1, US2, US3) can proceed in parallel or sequentially
  - P2 stories (US4, US5) depend on P1 completion for some integration
  - P3 story (US6) can start after P2 completion
- **Polish (Phase 9)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P1)**: Can start after US1 completion - Integrates with Logger from US1
- **User Story 3 (P1)**: Can start after US1 completion - Extends Logger with async queue
- **User Story 4 (P2)**: Can start after US1 completion - Extends FileManager
- **User Story 5 (P2)**: Can start after US1 completion - Integrates TagFilter
- **User Story 6 (P3)**: Can start after US4 completion - Archives files created by FileManager

### Within Each User Story

- Unit tests [P] can run in parallel
- Implementation tasks follow dependency order (headers → implementations → C bridge)
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel (T003, T004, T005)
- All Foundational tasks marked [P] can run in parallel (T007, T008, T009, T012, T017)
- P1 stories (US1, US2, US3) can be worked on in parallel by different developers
- P2 stories (US4, US5) can be worked on in parallel after P1 completion
- All unit tests for a user story marked [P] can run in parallel

---

## Parallel Example: User Story 1

```bash
# Launch unit tests for User Story 1 together:
Task: "Create unit test for log level filtering in tests/unit/test_logger.cpp"
Task: "Create unit test for LogEntry formatting in tests/unit/test_log_entry.cpp"

# Launch parallel implementation tasks for User Story 1:
Task: "Create Logger class with SetLogLevel method in src/include/speckit/log/logger.h"
Task: "Create FileManager class for basic file operations in src/include/speckit/log/file_manager.h"
```

---

## Parallel Example: P1 User Stories (Team of 3 developers)

```bash
# Once Foundational phase (Phase 2) is complete, work on all P1 stories in parallel:
Developer A: Tasks T016-T026 (User Story 1 - Basic logging)
Developer B: Tasks T027-T031 (User Story 2 - Process isolation)
Developer C: Tasks T032-T042 (User Story 3 - Async logging)
```

---

## Implementation Strategy

### MVP First (User Stories 1-3 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1 (Basic logging)
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Complete Phase 4: User Story 2 (Process isolation)
6. **STOP and VALIDATE**: Test multi-process logging
7. Complete Phase 5: User Story 3 (Async logging)
8. **STOP and VALIDATE**: Test <1ms latency and throughput
9. Deploy/demo P1 MVP (all three P1 stories complete)

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Demo basic logging
3. Add User Story 2 → Test independently → Demo process isolation
4. Add User Story 3 → Test independently → Demo async performance
5. Add User Story 4 → Test independently → Demo file rotation
6. Add User Story 5 → Test independently → Demo tag filtering
7. Add User Story 6 → Test independently → Demo archiving
8. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (Basic logging)
   - Developer B: User Story 2 (Process isolation)
   - Developer C: User Story 3 (Async logging)
3. After P1 completion:
   - Developer A: User Story 4 (File rotation)
   - Developer B: User Story 5 (Tag filtering)
   - Developer C: Polish & testing
4. Developer A/B: User Story 6 (Archiving) + Final polish
5. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- P1 stories (US1, US2, US3) form the core MVP
- P2 stories (US4, US5) add production-grade features
- P3 story (US6) adds archiving capability
- Tests are included as performance and integration tests are critical for this project
- Performance requirements (<1ms latency, 100K logs, 24h stability) must be validated
- Crash safety and data preservation are critical constitutional requirements
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Verify all P1 stories work together before proceeding to P2 features
