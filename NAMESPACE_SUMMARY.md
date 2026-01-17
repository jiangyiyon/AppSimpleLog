# 命名空间重构总结

## 变更概述

将 `log_level.h` 中的定义包裹到 `SpeckitLog` 命名空间，然后使用 `using` 声明将符号引入全局作用域。

## 修改的文件

### 核心头文件
- ✅ `src/include/speckit/log/log_level.h` - 添加 `SpeckitLog` 命名空间

### 源文件
- ✅ `src/src/async_logger.cpp`
- ✅ `src/src/async_queue.cpp`
- ✅ `src/src/crash_handler.cpp`
- ✅ `src/src/file_manager.cpp`
- ✅ `src/src/log_buffer.cpp`
- ✅ `src/src/log_entry.cpp`
- ✅ `src/src/logger.cpp`
- ✅ `src/src/tag_filter.cpp`

### 头文件
- ✅ `src/include/speckit/log/log_entry.h`
- ✅ `src/include/speckit/log/log_buffer.h`
- ✅ `src/include/speckit/log/tag_filter.h`
- ✅ `src/include/speckit/log/file_manager.h`
- ✅ `src/include/speckit/log/crash_handler.h`
- ✅ `src/include/speckit/log/async_queue.h`
- ✅ `src/include/speckit/log/async_logger.h`
- ✅ `src/include/speckit/log/logger.h`
- ✅ `src/include/speckit/log/features.h`

### C Bridge
- ✅ `cbridge/speckit_logger.h` - 更新类型为 `SpeckitLog::LogLevel`

### 测试文件
- ✅ 所有测试文件已更新

## 命名空间结构

### log_level.h
```cpp
namespace SpeckitLog {
    enum class LogLevel : int8_t { ... };
    inline bool shouldLog(LogLevel, LogLevel);
    constexpr const char* levelToString(LogLevel);
}

// 引入全局作用域
using SpeckitLog::LogLevel;
using SpeckitLog::shouldLog;
using SpeckitLog::levelToString;
```

### 其他头文件
所有类和函数保持全局命名空间（无前缀）。

## 使用方式

### 代码中
```cpp
#include "speckit/log/log_level.h"

// 可以直接使用 LogLevel（已通过 using 引入）
LogLevel level = LogLevel::kLogLevelInfo;

// 或者显式指定命名空间
SpeckitLog::LogLevel level = SpeckitLog::LogLevel::kLogLevelInfo;
```

### C API (cbridge)
```c
// C bridge 自动处理类型转换
SpeckitLogger* logger = speckit_logger_create("app");
speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "TAG", "msg");
```

## 类型定义

以下类型在全局作用域可用：
- `LogLevel` (枚举类型)
- `ProcessIdType` (从 platform.h)
- `ThreadIdType` (从 platform.h)

## 函数

以下函数在全局作用域可用：
- `shouldLog(LogLevel, LogLevel)` - 检查日志级别
- `levelToString(LogLevel)` - 转换为字符串
- `formatLogEntry(const LogEntry&)` - 格式化日志条目

## 类

以下类在全局作用域：
- `LogEntry` - 日志条目结构
- `RingBuffer` - 环形缓冲区
- `AsyncQueue` - 异步队列
- `TagFilter` - 标签过滤器
- `FileManager` - 文件管理器
- `CrashHandler` - 崩溃处理器
- `AsyncLogger` - 异步日志器
- `Logger` - 同步日志器

## 命名空间策略

### 为什么使用 `using` 而不是嵌套命名空间？

1. **C API 兼容性** - C bridge 需要明确的类型名称
2. **避免深层嵌套** - 防止代码中出现 `SpeckitLog::Logger::` 等长名称
3. **向后兼容** - 现有代码只需添加头文件即可工作
4. **清晰性** - 主要符号可见，辅助函数可以引用命名空间

### 命名空间层次

```
SpeckitLog        // LogLevel 等辅助类型和函数
全局             // 所有主要类
```

## 编译验证

要验证更改：

```bash
# 清理
rd /s /q build

# 重新配置
configure_vs2026.bat

# 编译
cmake --build build\debug --config Debug
```

## 常见问题

### Q: 为什么 `LogLevel` 可以直接使用？
A: 因为 `using SpeckitLog::LogLevel;` 将其引入全局作用域。

### Q: C API 如何处理类型转换？
A: `speckit_logger.h` 中的 `toCppLogLevel()` 和 `toCLogLevel()` 函数处理转换。

### Q: 如果发生命名冲突怎么办？
A: 修改 `using` 声明，例如 `using SpeckitLog::LogLevel as MyLogLevel;`

## 下一步

1. 编译项目
2. 运行测试
3. 检查链接错误
