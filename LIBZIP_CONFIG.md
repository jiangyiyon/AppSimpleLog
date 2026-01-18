# libzip 配置说明 (Windows x64)

## 概述

本项目已配置为使用本地下载的 libzip 库进行 ZIP 压缩归档。

## 目录结构

确保第三方库位于正确的目录：

```
AppSimpleLog/
├── third_party/
│   ├── libzip_x64-windows/        # libzip 库 (已下载)
│   │   ├── include/
│   │   │   ├── zip.h
│   │   │   └── zipconf.h
│   │   └── lib/
│   │       └── zip.lib
│   └── zlib_x64-windows/         # zlib 库 (依赖项)
│       ├── include/
│       │   ├── zlib.h
│       │   └── zconf.h
│       └── lib/
│           └── zlib.lib
```

## CMake 配置

### 自动配置

CMakeLists.txt 已配置为自动检测和使用本地的 libzip 和 zlib：

```cmake
# Windows 平台配置
if(WIN32)
    # 设置 libzip 路径
    set(LIBZIP_ROOT "${CMAKE_SOURCE_DIR}/third_party/libzip_x64-windows")
    set(LIBZIP_INCLUDE_DIR "${LIBZIP_ROOT}/include")
    set(LIBZIP_LIB_DIR "${LIBZIP_ROOT}/lib")
    set(LIBZIP_LIB "${LIBZIP_LIB_DIR}/zip.lib")

    # 检查文件是否存在
    if(EXISTS "${LIBZIP_LIB}" AND EXISTS "${LIBZIP_INCLUDE_DIR}/zip.h")
        message(STATUS "Found local libzip: ${LIBZIP_ROOT}")
        target_include_directories(SpeckitLog PRIVATE ${LIBZIP_INCLUDE_DIR})
        target_link_libraries(SpeckitLog PRIVATE ${LIBZIP_LIB})
    else()
        message(FATAL_ERROR "libzip not found...")
    endif()

    # 设置 zlib 路径 (libzip 依赖 zlib)
    set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/third_party/zlib_x64-windows")
    # ... (类似配置)
endif()
```

### 依赖关系

```
SpeckitLog (静态库)
├── src/src/archive.cpp
│   └── #include <zip.h> → third_party/libzip_x64-windows/include/zip.h
└── 链接到
    ├── zip.lib (libzip)
    └── zlib.lib (zlib, libzip 依赖)

SpeckitBridge (动态库)
├── cbridge/speckit_logger.cpp
└── 链接到
    └── SpeckitLog
```

## 构建项目

### 1. 配置 CMake

```bash
cd e:\APPLogByAI\AppSimpleLog

# 创建构建目录
mkdir build
cd build

# 配置 CMake (x64 Release)
cmake .. -G "Visual Studio 17 2022" -A x64
```

### 2. 验证配置输出

CMake 应该输出类似以下信息：

```
-- Found local libzip: e:/APPLogByAI/AppSimpleLog/third_party/libzip_x64-windows
-- Found local zlib: e:/APPLogByAI/AppSimpleLog/third_party/zlib_x64-windows
Configuration summary:
  C++ Standard: 20
  Build type: (blank for multi-config)
  Platform: Windows
  Architecture: x64
  Build testing: ON
```

### 3. 编译

```bash
# 编译 Release 版本
cmake --build . --config Release

# 或使用 Visual Studio 打开解决方案
start AppSimpleLog.sln
```

### 4. 验证链接

编译成功后，检查链接器输出应该包含：

```
zip.lib
zlib.lib
```

## 故障排除

### 问题 1: "zip.h not found"

**原因**: libzip 路径配置错误或文件缺失

**解决方案**:
1. 确认目录结构正确：
   ```
   third_party/libzip_x64-windows/include/zip.h
   third_party/libzip_x64-windows/lib/zip.lib
   ```

2. 检查 CMake 输出中的错误消息：
   ```
   -- Found local libzip: .../third_party/libzip_x64-windows
   ```
   如果看到 `FATAL_ERROR`，说明路径或文件缺失。

### 问题 2: "unresolved external symbol zip_open"

**原因**: libzip 库未正确链接

**解决方案**:
1. 检查 CMakeLists.txt 中的链接配置
2. 确认 `zip.lib` 文件存在于 `third_party/libzip_x64-windows/lib/`
3. 重新运行 CMake 配置：
   ```bash
   rm -rf CMakeCache.txt CMakeFiles/
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

### 问题 3: "unresolved external symbol inflate"

**原因**: zlib 库未正确链接（libzip 依赖 zlib）

**解决方案**:
1. 确认 zlib 文件存在：
   ```
   third_party/zlib_x64-windows/lib/zlib.lib
   third_party/zlib_x64-windows/include/zlib.h
   ```
2. 检查 CMake 输出中的 zlib 检测消息
3. 重新配置和构建

### 问题 4: 运行时错误 "无法找到 zip.dll"

**原因**: 运行时缺少 DLL 文件

**解决方案**:
1. 如果 libzip 需要 DLL，将其复制到可执行文件目录：
   ```
   copy third_party\libzip_x64-windows\bin\*.dll build\Release\
   ```
2. 或将 `third_party/libzip_x64-windows/bin` 添加到 PATH 环境变量

### 问题 5: 归档功能不工作

**原因**: 编译时未正确链接库，或运行时库不可用

**解决方案**:
1. 检查链接器输出中是否包含 `zip.lib`
2. 运行时测试：
   ```bash
   # 在项目根目录
   dir third_party\libzip_x64-windows\lib\zip.lib
   ```
3. 使用 Dependency Walker 或类似工具检查依赖关系

## 验证测试

运行单元测试验证归档功能：

```bash
# 编译测试
cmake --build . --config Release

# 运行归档测试
cd build\Release
unit_tests --gtest_filter="ArchiveTest.*"
```

预期输出：
```
[==========] Running 8 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 8 tests from ArchiveTest
[ RUN      ] ArchiveTest.CreateArchive_CreatesZipFile
[       OK ] ArchiveTest.CreateArchive_CreatesZipFile (XX ms)
...
[==========] 8 tests from 1 test suite ran. (XX ms total)
[  PASSED  ] 8 tests.
```

## 性能测试

使用提供的性能测试验证压缩效果：

```bash
performance_tests --gtest_filter="ArchiveTest.*"
```

预期看到类似输出：
```
Archive test completed
Archive size: 30 KB
Original size: 300 KB
Compression ratio: 90%
```

## CMake 调试

如果遇到问题，启用详细输出：

```bash
# 删除缓存
rm -rf CMakeCache.txt CMakeFiles/

# 重新配置（详细模式）
cmake .. -G "Visual Studio 17 2022" -A x64 --trace --debug-output

# 查看配置日志
type CMakeFiles/CMakeOutput.log
```

## 手动配置路径（可选）

如果需要自定义库路径，修改 CMakeLists.txt 第 54-73 行：

```cmake
# 自定义 libzip 路径
set(LIBZIP_ROOT "C:/custom/path/to/libzip")

# 自定义 zlib 路径
set(ZLIB_ROOT "C:/custom/path/to/zlib")
```

或通过命令行指定：

```bash
cmake .. -DLIBZIP_ROOT="C:/custom/path" -DZLIB_ROOT="C:/custom/path"
```

## 总结

✅ **已完成的配置**:
1. libzip 库路径配置
2. zlib 依赖库配置
3. 编译器包含目录设置
4. 链接器库链接配置
5. SpeckitBridge 依赖配置
6. 详细错误消息和状态输出

📋 **下一步**:
1. 运行 `cmake .. -G "Visual Studio 17 2022" -A x64`
2. 验证 CMake 输出中显示 "Found local libzip"
3. 编译项目：`cmake --build . --config Release`
4. 运行测试验证归档功能

📖 **相关文档**:
- [libzip 集成说明](./LIBZIP_INTEGRATION.md)
- [归档测试](./tests/unit/test_archive.cpp)
- [技术设计文档](./specs/001-async-logging-module/TECHNICAL_DESIGN.md)

---

**更新日期**: 2026-01-17
**版本**: 1.0
**平台**: Windows x64
