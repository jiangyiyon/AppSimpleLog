# AppSimpleLog 构建指南

## 当前配置状态

项目已配置为使用 **Visual Studio 2026** 生成器进行Windows编译。

**重要**：首次构建前，需要确保VS工具链可用。

## 修复的问题

### 1. CMakeLists.txt 语法错误
- ✅ 修复了源文件列表（移除不存在的`archive.cpp`，添加`async_logger.cpp`和`async_queue.cpp`）
- ✅ 修复了嵌套`if`语句的`endif()`缺失问题
- ✅ 将GTest依赖改为可选（未安装时不影响库编译）

### 2. pthread.h 依赖问题
- ✅ 将`pthread_self()`替换为C++20标准的`std::thread::id`
- ✅ 添加必要的头文件 `<thread>` 和 `<mutex>`

### 3. 构建配置
- ✅ 切换到Visual Studio 2022生成器（更兼容Windows）
- ✅ 配置了Debug/Release双构建配置
- ✅ 更新了所有路径以匹配VS输出结构

## 在VS Code中构建

### 首次使用前准备

1. 安装 **CMake Tools** 扩展（VS Code会推荐）
2. 安装 **C/C++** 扩展
3. 确保Visual Studio 2026已安装，且包含"C++桌面开发"工作负载

### 方法1：使用CMake Tools扩展（推荐）

1. 打开项目后，底部状态栏会显示CMake Tools
2. 点击状态栏的 **Debug/Release** 切换构建类型
3. 点击 **Build** 按钮开始构建
4. 或者按 `F7` 快捷键构建

### 方法2：使用任务

1. 按 `Ctrl+Shift+P`
2. 输入 "Tasks: Run Task"
3. 选择任务：
   - `CMake: Configure (Debug)` - 配置Debug构建
   - `CMake: Build (Debug)` - 编译Debug版本
   - `CMake: Build Library Only` - 只编译库

### 方法3：使用快捷键

1. 按 `Ctrl+Shift+B` 打开任务列表
2. 选择 `CMake: Build (Debug)`

### 方法4：在VS中打开项目（备选）

如果VS Code构建遇到问题，可以直接用Visual Studio：

1. 打开 `build/debug/AppSimpleLog.sln`
2. 在Visual Studio 2026中打开
3. 选择 Debug 配置
4. 按 `Ctrl+Shift+B` 生成解决方案

## 手动构建（命令行）

### 配置
```powershell
cd E:\APPLogByAI\AppSimpleLog
cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTING=ON
```

### 编译
```powershell
cmake --build build\debug --config Debug
```

**注意**：如果在命令行构建失败，可能需要：
1. 打开 "Developer Command Prompt for VS 2026"
2. 在该命令提示符中运行上述命令

### 只编译库
```powershell
cmake --build build\debug --config Debug --target SpeckitLog SpeckitBridge
```

### 清理
```powershell
cmake --build build\debug --target clean
```

## 输出位置

- **库文件**：`build\debug\Debug\SpeckitLog.lib`
- **C Bridge DLL**：`build\debug\Debug\SpeckitBridge.dll`
- **头文件**：自动安装到`install\include\`
- **测试可执行文件**：`build\debug\Debug\unit_tests.exe`（如果GTest已安装）

## 调试配置

按 `F5` 启动调试，可用的调试配置：
- `Debug: Unit Tests` - 调试单元测试
- `Debug: Integration Tests` - 调试集成测试
- `Debug: Specific Unit Test` - 调试特定测试用例
- `Debug: Performance Tests` - 调试性能测试（Release模式）

## GTest 安装（可选）

如果要运行测试，需要安装GTest：

### 使用 Vcpkg（推荐）
```powershell
vcpkg install gtest:x64-windows
```

### 手动安装
1. 从 https://github.com/google/googletest 下载
2. 编译并安装到 CMake 能找到的位置

## 常见问题

### CMake 配置失败
- 确保已安装 Visual Studio 2022（包含C++工作负载）
- 检查 CMake 版本：`cmake --version`（需要 3.20+）

### 编译错误
- 清理构建目录：删除 `build` 文件夹后重新配置
- 检查C++20支持：确保编译器支持C++20

### 测试失败
- 确认GTest已正确安装：`find_package(GTest)` 应该成功
- 检查测试文件是否存在：`tests/` 目录

## 文件结构

```
AppSimpleLog/
├── .vscode/               # VS Code 配置
├── build/                 # 构建输出
│   └── debug/            # Debug 配置
│       ├── Debug/        # Debug 可执行文件和库
│       └── Release/      # Release 可执行文件和库
├── src/                  # 源代码
│   ├── include/         # 公共头文件
│   └── src/             # 实现文件
├── cbridge/             # C 接口
├── tests/               # 测试文件
└── install/             # 安装输出
```

## 下一步

1. 运行 `CMake: Configure (Debug)` 任务
2. 运行 `CMake: Build (Debug)` 任务
3. 检查编译输出和可能的错误
4. 如果安装了GTest，运行测试

需要帮助？查看 `README.md` 获取项目概述。
