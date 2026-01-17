# GTest 设置指南

## 方式1：使用集成源码（推荐，最简单）

### 步骤：

1. **下载 GTest 源码**
   ```bash
   download_gtest.bat
   ```
   这会下载 GTest v1.14.0 到 `third_party/googletest`

2. **重新配置项目**
   ```bash
   configure_vs2026.bat
   ```
   CMakeLists.txt 已自动配置为使用集成的 GTest 源码

3. **构建项目**
   ```bash
   cmake --build build\debug --config Debug
   ```

4. **运行测试**
   ```bash
   build\debug\Debug\unit_tests.exe
   ```

## 方式2：使用 vcpkg 安装（高级用户）

如果你有 vcpkg：

```bash
vcpkg install gtest:x64-windows
```

然后设置 CMAKE_TOOLCHAIN_FILE：
```bash
cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake ^
    -DBUILD_TESTING=ON
```

## 方式3：手动下载

1. 访问：https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
2. 下载并解压到：`third_party/googletest`
3. 运行：`configure_vs2026.bat`

## 验证安装

运行以下命令检查 GTest 是否可用：

```bash
cmake -S . -B build\debug -LA 2>&1 | findstr "GTest"
```

应该看到类似输出：
```
-- GTest_FOUND: TRUE
```

## 故障排除

### 错误：GTest not found

确保：
1. 已运行 `download_gtest.bat`
2. `third_party/googletest` 目录存在
3. 目录结构正确：
   ```
   third_party/
   └── googletest/
       ├── googletest/
       │   ├── include/
       │   │   └── gtest/
       │   │       └── gtest.h
       │   └── src/
       │       └── gtest-all.cc
       └── googlemock/
           ├── include/
           │   └── gmock/
           │       └── gmock.h
           └── src/
               └── gmock-all.cc
   ```

### 错误：编译错误

如果测试编译失败，检查：
1. CMake 版本是否 >= 3.16
2. 编译器是否支持 C++20
3. 所有测试源文件是否存在于 `tests/` 目录

### 错误：链接错误

确保：
1. CMakeLists.txt 中的 GTest_SOURCE 变量定义正确
2. GTest 源文件路径正确

## 当前状态

- ✅ CMakeLists.txt 已更新支持集成 GTest
- ✅ 创建了自动下载脚本
- ✅ 配置了测试目标

## 下一步

运行以下命令开始：

```bash
download_gtest.bat
configure_vs2026.bat
cmake --build build\debug --config Debug
```

祝测试愉快！
