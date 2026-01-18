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
    src/src/tag_filter.cpp
    src/src/archive.cpp
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
target_link_libraries(SpeckitBridge PRIVATE SpeckitLog)

# Define export macro for Windows
if(WIN32)
    target_compile_definitions(SpeckitBridge PRIVATE SPECKITBRIDGE_EXPORTS)
endif()

# Platform-specific linking
if(WIN32)
    # Use local libzip library from third_party
    set(LIBZIP_ROOT "${CMAKE_SOURCE_DIR}/third_party/libzip_x64-windows")
    set(LIBZIP_INCLUDE_DIR "${LIBZIP_ROOT}/include")
    set(LIBZIP_LIB_DIR "${LIBZIP_ROOT}/lib")
    set(LIBZIP_LIB "${LIBZIP_LIB_DIR}/zip.lib")

    # Check if libzip files exist
    if(EXISTS "${LIBZIP_LIB}" AND EXISTS "${LIBZIP_INCLUDE_DIR}/zip.h")
        message(STATUS "Found local libzip: ${LIBZIP_ROOT}")
        target_include_directories(SpeckitLog PRIVATE ${LIBZIP_INCLUDE_DIR})
        target_link_libraries(SpeckitLog PRIVATE ${LIBZIP_LIB})
        # SpeckitBridge also needs libzip for archive functionality
        target_link_libraries(SpeckitBridge PRIVATE ${LIBZIP_LIB})
    else()
        message(FATAL_ERROR "libzip not found in ${LIBZIP_ROOT}. Please download libzip to third_party/libzip_x64-windows/")
    endif()

    # Also link zlib (required by libzip)
    set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/third_party/zlib_x64-windows")
    set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
    set(ZLIB_LIB_DIR "${ZLIB_ROOT}/lib")
    set(ZLIB_LIB "${ZLIB_LIB_DIR}/zlib.lib")

    if(EXISTS "${ZLIB_LIB}")
        message(STATUS "Found local zlib: ${ZLIB_ROOT}")
        target_include_directories(SpeckitLog PRIVATE ${ZLIB_INCLUDE_DIR})
        target_link_libraries(SpeckitLog PRIVATE ${ZLIB_LIB})
        # SpeckitBridge also needs zlib
        target_link_libraries(SpeckitBridge PRIVATE ${ZLIB_LIB})
    else()
        message(WARNING "zlib not found in ${ZLIB_ROOT}. libzip may fail to link.")
    endif()
else()
    target_link_libraries(SpeckitLog PRIVATE pthread z)
    # Add libzip library for macOS/Linux
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBZIP IMPORTED_TARGET libzip)
        if(TARGET PkgConfig::LIBZIP)
            target_link_libraries(SpeckitLog PRIVATE PkgConfig::LIBZIP)
        else()
            # Fallback: try to find libzip manually
            find_library(LIBZIP_LIB zip)
            if(LIBZIP_LIB)
                target_link_libraries(SpeckitLog PRIVATE ${LIBZIP_LIB})
            else()
                message(WARNING "libzip not found. Archive functionality may not work. Please install libzip.")
            endif()
        endif()
    endif()
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
            tests/unit/test_archive.cpp
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
            target_link_libraries(unit_tests PRIVATE SpeckitLog)
        else()
            # Use installed GTest
            add_executable(unit_tests ${UNIT_TEST_SOURCES})
            target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main SpeckitLog)
        endif()
        
        # Add libzip and zlib to unit tests (needed for test_archive.cpp)
        if(WIN32 AND EXISTS "${LIBZIP_LIB}" AND EXISTS "${ZLIB_LIB}")
            target_include_directories(unit_tests PRIVATE ${LIBZIP_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
            target_link_libraries(unit_tests PRIVATE ${LIBZIP_LIB} ${ZLIB_LIB})
        endif()
        
        add_test(NAME unit_tests COMMAND unit_tests)

        # Integration tests executable
        if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
            add_executable(integration_tests ${INTEGRATION_TEST_SOURCES} ${GTEST_SOURCES} ${GTEST_MAIN_SOURCE})
            target_link_libraries(integration_tests PRIVATE SpeckitLog SpeckitBridge)
        else()
            add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
            target_link_libraries(integration_tests PRIVATE GTest::gtest GTest::gtest_main SpeckitLog SpeckitBridge)
        endif()
        
        # Add libzip and zlib to integration tests (needed for archive functionality)
        if(WIN32 AND EXISTS "${LIBZIP_LIB}" AND EXISTS "${ZLIB_LIB}")
            target_include_directories(integration_tests PRIVATE ${LIBZIP_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
            target_link_libraries(integration_tests PRIVATE ${LIBZIP_LIB} ${ZLIB_LIB})
        endif()
        
        add_test(NAME integration_tests COMMAND integration_tests)

        # Performance tests executable
        if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest/gtest.h")
            add_executable(performance_tests ${PERFORMANCE_TEST_SOURCES} ${GTEST_SOURCES} ${GTEST_MAIN_SOURCE})
            target_link_libraries(performance_tests PRIVATE SpeckitLog)
        else()
            add_executable(performance_tests ${PERFORMANCE_TEST_SOURCES})
            target_link_libraries(performance_tests PRIVATE GTest::gtest GTest::gtest_main SpeckitLog)
        endif()
        
        # Add libzip and zlib to performance tests (needed for archive functionality)
        if(WIN32 AND EXISTS "${LIBZIP_LIB}" AND EXISTS "${ZLIB_LIB}")
            target_include_directories(performance_tests PRIVATE ${LIBZIP_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
            target_link_libraries(performance_tests PRIVATE ${LIBZIP_LIB} ${ZLIB_LIB})
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