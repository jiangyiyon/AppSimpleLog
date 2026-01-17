# AppSimpleLog - VS Code 开发环境配置指南

本文档介绍如何在 VS Code 中配置和使用 AppSimpleLog 项目的编译、调试和测试环境。

## 前置要求

### 必需工具
- **Visual Studio 2022** (Community/Professional/Enterprise)
  - 安装"使用 C++ 的桌面开发"工作负载
- **CMake** (3.20 或更高版本)
- **Ninja** 构建工具
- **Git**

### VS Code 扩展
安装以下推荐扩展（`.vscode/extensions.json` 中已配置）：
```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
code --install-extension xaver.clang-format
code --install-extension matepek.vscode-test-explorer
```

或者在 VS Code 中打开扩展面板，搜索并安装上述扩展。

## 快速开始

### 1. 安装 Ninja

**Windows (使用 Chocolatey):**
```powershell
choco install ninja
```

**Windows (手动安装):**
1. 从 [Ninja GitHub](https://github.com/ninja-build/ninja/releases) 下载最新版本
2. 解压到 `C:\ninja` 或其他目录
3. 将 Ninja 路径添加到系统 PATH

### 2. 配置 CMake

首次打开项目时，CMake Tools 扩展会自动配置。如果未自动配置：

1. 打开命令面板 (`Ctrl+Shift+P`)
2. 选择 "CMake: Configure"
3. 选择 "Ninja Multi-Config" 作为生成器
4. 等待配置完成

### 3. 构建项目

使用任务（`Ctrl+Shift+B`）：
- `CMake: Build (Debug)` - 默认构建任务
- `CMake: Build (Release)` - Release 版本构建
- `CMake: Build Library Only` - 仅构建库文件

或使用 CMake Tools 面板：
1. 点击底部状态栏的 `Debug` / `Release` 切换构建配置
2. 点击 "Build" 按钮构建项目

## 运行测试

### 通过 VS Code 任务

#### 运行所有单元测试
- 快捷键: `Ctrl+Shift+P` → "Tasks: Run Task" → "Test: Run Unit Tests"

#### 运行特定测试类型
- **单元测试**: "Test: Run Unit Tests"
- **集成测试**: "Test: Run Integration Tests"
- **性能测试**: "Test: Run Performance Tests"
- **所有测试**: "Test: Run All Tests (CMake)"

### 通过调试器运行

1. 按 `F5` 或点击调试面板
2. 选择测试配置：
   - `Debug: Unit Tests` - 运行所有单元测试
   - `Debug: Unit Tests (Stop at main)` - 在 main 函数处暂停
   - `Debug: Specific Unit Test` - 运行特定测试（需要修改 `--gtest_filter`）
   - `Debug: Integration Tests` - 运行集成测试
   - `Debug: Performance Tests` - 运行性能测试（Release 模式）

### 通过命令行

```powershell
# 进入构建目录
cd build

# 运行单元测试 (Debug)
.\Debug\unit_tests.exe --gtest_color=yes

# 运行集成测试
.\Debug\integration_tests.exe --gtest_color=yes

# 运行性能测试 (Release)
.\Release\performance_tests.exe --gtest_color=yes

# 运行所有测试
ctest --config Debug --output-on-failure
```

## 调试配置

### 断点设置
- 在代码行号左侧点击设置断点
- 支持条件断点：右键 → "Add Conditional Breakpoint"

### 调试特定测试

修改 `.vscode/launch.json` 中的 `--gtest_filter` 参数：
```json
"args": [
    "--gtest_filter=LogEntryTest.FormatTimestamp"  // 仅运行特定测试
],
```

支持通配符：
```json
"args": [
    "--gtest_filter=LogEntryTest.*"  // 运行 LogEntryTest 所有测试
],
```

### 查看测试结果
测试运行时，VS Code 会输出测试结果，包括：
- ✅ 通过的测试
- ❌ 失败的测试（带错误信息）
- ⏱️ 执行时间

## 代码格式化

项目使用 clang-format 进行代码格式化。

### 自动格式化
保存文件时自动格式化（已在 `.vscode/settings.json` 中配置）

### 手动格式化
- 快捷键: `Shift+Alt+F`
- 命令面板: `Ctrl+Shift+P` → "Format Document"

### 格式化规则
格式化规则在 `.clang-format` 文件中定义，主要规则：
- 基于 Google 风格
- 缩进: 4 空格
- 行宽限制: 100 字符
- 指针对齐: 左对齐 (`int* ptr`)

## CMake 可用任务

| 任务名称 | 说明 |
|---------|------|
| CMake: Configure (Debug) | 配置 Debug 构建 |
| CMake: Configure (Release) | 配置 Release 构建 |
| CMake: Build (Debug) | 构建 Debug 版本（默认） |
| CMake: Build (Release) | 构建 Release 版本 |
| CMake: Build Library Only | 仅构建库文件 |
| CMake: Build Unit Tests | 构建单元测试 |
| CMake: Build Integration Tests | 构建集成测试 |
| CMake: Build Performance Tests | 构建性能测试 |
| Test: Run Unit Tests | 运行单元测试 |
| Test: Run Integration Tests | 运行集成测试 |
| Test: Run Performance Tests | 运行性能测试 |
| Test: Run All Tests (CMake) | 运行所有测试 |
| CMake: Clean | 清理构建产物 |
| CMake: Reconfigure and Rebuild | 完全重新构建 |
| CMake: Install | 安装到 install 目录 |

## 项目结构

```
AppSimpleLog/
├── .vscode/              # VS Code 配置
│   ├── settings.json      # 编辑器设置
│   ├── tasks.json         # 构建和测试任务
│   ├── launch.json        # 调试配置
│   ├── c_cpp_properties.json # C++ IntelliSense 配置
│   └── extensions.json   # 推荐扩展
├── src/                  # 源代码
│   ├── include/          # 头文件
│   └── src/             # 实现文件
├── cbridge/              # C 桥接层
├── tests/                # 测试
│   ├── unit/            # 单元测试
│   ├── integration/     # 集成测试
│   └── performance/    # 性能测试
├── specs/               # 技术规格文档
├── CMakeLists.txt       # CMake 配置
├── .clang-format       # 代码格式化规则
└── .gitignore         # Git 忽略规则
```

## 常见问题

### Q: CMake 配置失败
**A:** 确保已安装 Visual Studio 2022 和 CMake，并在 VS Code 中选择了正确的编译器。

### Q: 构建时找不到头文件
**A:** 检查 `.vscode/c_cpp_properties.json` 中的 `includePath` 是否正确。

### Q: 测试运行失败
**A:**
1. 确保已成功构建测试目标
2. 检查测试输出中的错误信息
3. 在调试器中运行测试，设置断点查看详细信息

### Q: Ninja 未找到
**A:** 确保 Ninja 已安装并添加到系统 PATH。在 PowerShell 中验证：
```powershell
ninja --version
```

### Q: 如何添加新的测试
**A:**
1. 在 `tests/unit/` 或 `tests/integration/` 创建新的测试文件
2. 在 `CMakeLists.txt` 中添加到相应的源文件列表
3. 重新构建测试

### Q: 代码格式化不生效
**A:**
1. 确保安装了 clang-format 扩展
2. 检查 `.clang-format` 文件是否在项目根目录
3. 在设置中确认 `"C_Cpp.formatting": "default"`

## 性能测试

性能测试使用 Release 构建，以获得准确的性能数据。

```powershell
# 构建 Release 版本
cmake --build build --config Release --target performance_tests

# 运行性能测试
.\build\Release\performance_tests.exe --gtest_color=yes
```

## 清理构建

清理所有构建产物：
```powershell
# 通过任务
Ctrl+Shift+P → "Tasks: Run Task" → "CMake: Clean"

# 或手动删除
Remove-Item -Recurse -Force build
```

## 更多资源

- [CMake Tools 文档](https://vector-of-bool.github.io/docs/vscode-cmake-tools/)
- [VS Code C/C++ 文档](https://code.visualstudio.com/docs/cpp/)
- [GoogleTest 文档](https://google.github.io/googletest/)
- [Clang-Format 文档](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)

## 贡献指南

1. Fork 项目
2. 创建特性分支
3. 编写代码并添加测试
4. 运行所有测试确保通过
5. 提交 Pull Request

**代码提交前检查清单：**
- [ ] 代码通过 clang-format 格式化
- [ ] 所有单元测试通过
- [ ] 所有集成测试通过
- [ ] 代码遵循项目风格规范
- [ ] 添加了必要的文档注释
