cmake_minimum_required(VERSION 3.16)
project(AppSimpleLog VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/cbridge
)

# Source files for the logging library
set(LOG_LIB_SOURCES
    src/src/log_entry.cpp
    src/src/log_buffer.cpp
    src/src/file_manager.cpp
    src/src/async_logger.cpp
    src/src/async_queue.cpp
    src/src/crash_handler.cpp
)

# C bridge source files
set(CBRIDGE_SOURCES
    cbridge/speckit_logger.cpp
)

# Create the main logging library
add_library(SpeckitLog STATIC ${LOG_LIB_SOURCES})
target_include_directories(SpeckitLog PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

# Create the C bridge library
add_library(SpeckitBridge SHARED ${CBRIDGE_SOURCES})
target_include_directories(SpeckitBridge PUBLIC ${CMAKE_SOURCE_DIR}/cbridge)
target_link_libraries(SpeckitBridge SpeckitLog)

# Define export macro for Windows
if(WIN32)
    target_compile_definitions(SpeckitBridge PRIVATE SPECKITBRIDGE_EXPORTS)
endif()

# Platform-specific linking
if(WIN32)
    target_link_libraries(SpeckitLog PRIVATE)
else()
    target_link_libraries(SpeckitLog PRIVATE pthread z)
    target_link_libraries(SpeckitBridge PRIVATE pthread z)
endif()

# GoogleTest integration for testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()

    # Check if GTest source code is available
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
        set(GTEST_FOUND TRUE)

        # GTest source files
        set(GTEST_SOURCES
            third_party/googletest/googletest/src/gtest-all.cc
            third_party/googletest/googlemock/src/gmock-all.cc
        )

        # GTest main source (provides main function for tests)
        set(GTEST_MAIN_SOURCE
            third_party/googletest/googletest/src/gtest_main.cc
        )

        # GTest include directories
        include_directories(
            third_party/googletest/googletest/include
            third_party/googletest/googletest
            third_party/googletest/googlemock/include
            third_party/googletest/googlemock
        )
    else()
        find_package(GTest QUIET)
    endif()

    if(GTEST_FOUND)
        # Unit test sources - Updated to match actual files
        set(UNIT_TEST_SOURCES
            tests/unit/test_log_entry.cpp
            tests/unit/test_file_manager.cpp
            tests/unit/test_async_logging.cpp
            tests/unit/test_async_queue.cpp
            tests/unit/test_process_isolation.cpp
        )

        # Integration test sources - Updated to match actual files
        set(INTEGRATION_TEST_SOURCES
            tests/integration/test_async_logging.cpp
            tests/integration/test_multi_process.cpp
        )

        # Performance test sources - Updated to match actual files
        set(PERFORMANCE_TEST_SOURCES
            tests/performance/test_latency.cpp
            tests/performance/test_throughput.cpp
        )

        # Unit tests executable
        if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
            # Use integrated GTest source
            add_executable(unit_tests ${UNIT_TEST_SOURCES} ${GTEST_SOURCES} ${GTEST_MAIN_SOURCE})
            target_link_libraries(unit_tests SpeckitLog)
        else()
            # Use installed GTest
            add_executable(unit_tests ${UNIT_TEST_SOURCES})
            target_link_libraries(unit_tests GTest::gtest GTest::gtest_main SpeckitLog)
        endif()
        add_test(NAME unit_tests COMMAND unit_tests)

        # Integration tests executable
        if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
            add_executable(integration_tests ${INTEGRATION_TEST_SOURCES} ${GTEST_SOURCES} ${GTEST_MAIN_SOURCE})
            target_link_libraries(integration_tests SpeckitLog SpeckitBridge)
        else()
            add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
            target_link_libraries(integration_tests GTest::gtest GTest::gtest_main SpeckitLog SpeckitBridge)
        endif()
        add_test(NAME integration_tests COMMAND integration_tests)

        # Performance tests executable
        if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
            add_executable(performance_tests ${PERFORMANCE_TEST_SOURCES} ${GTEST_SOURCES} ${GTEST_MAIN_SOURCE})
            target_link_libraries(performance_tests SpeckitLog)
        else()
            add_executable(performance_tests ${PERFORMANCE_TEST_SOURCES})
            target_link_libraries(performance_tests GTest::gtest GTest::gtest_main SpeckitLog)
        endif()
        add_test(NAME performance_tests COMMAND performance_tests)
    else()
        message(WARNING "GTest not found. Tests will not be built. Please install GTest or set BUILD_TESTING=OFF")
        message(STATUS "To enable tests, run: download_gtest.bat")
    endif()
endif()

# Installation rules
install(TARGETS SpeckitLog SpeckitBridge
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY src/include/speckit/
    DESTINATION include/speckit
    FILES_MATCHING PATTERN "*.h"
)

install(FILES cbridge/speckit_logger.h
    DESTINATION include
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "AppSimpleLog")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP;TGZ")
include(CPack)

# Print configuration summary
message(STATUS "Configuration summary:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Build testing: ${BUILD_TESTING}")