# 技术方案说明书：高性能异步跨平台日志模块

**文档版本**: v1.1
**创建日期**: 2026-01-16
**更新日期**: 2026-01-16
**更新内容**: 移除标签过滤功能，仅保留标签透传
**项目**: AppSimpleLog
**功能模块**: 异步跨平台日志模块
**规格文档**: [spec.md](./spec.md)

---

## 目录

1. [项目概述](#1-项目概述)
2. [系统架构设计](#2-系统架构设计)
3. [核心模块设计](#3-核心模块设计)
4. [数据流设计](#4-数据流设计)
5. [接口设计](#5-接口设计)
6. [性能优化策略](#6-性能优化策略)
7. [异常处理机制](#7-异常处理机制)
8. [测试策略](#8-测试策略)
9. [部署与运维](#9-部署与运维)
10. [风险评估与缓解](#10-风险评估与缓解)

---

## 1. 项目概述

### 1.1 项目背景

开发一个高性能、异步、跨平台的日志库，用于支持Windows和macOS平台（x64/arm64架构）的应用程序日志记录需求。该日志库需要提供多进程隔离、异步文件滚动、标签透传（用于后续日志查看器过滤）、ZIP归档等功能，同时满足严格性能指标（<1ms调用延迟，100K+日志吞吐量）。

### 1.2 技术栈

- **编程语言**: C++20
- **标准库**: C++20 Standard Library (std::format, std::source_location, std::atomic, std::jthread)
- **第三方依赖**: 无（零依赖设计）
- **跨平台支持**: Windows (x64/arm64), macOS (x64/arm64)
- **接口方式**: C-Bridge模式（对外暴露纯C接口，内部C++实现）
- **测试框架**: GoogleTest
- **构建系统**: CMake

### 1.3 核心目标

| 目标 | 指标 |
|------|------|
| 调用延迟 | < 1ms |
| 日志吞吐量 | 100K+ 条/秒 |
| 并发支持 | 10+ 线程 |
| 内存占用 | < 200MB |
| 崩溃数据保存率 | ≥ 90% |
| 文件滚动时间 | < 100ms |
| 归档性能 | < 5s (10MB) |

---

## 2. 系统架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (C/C++/ObjC)                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    C-Bridge API 层 (extern "C")              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │ • speckit_logger_create()                            │   │
│  │ • speckit_logger_log()                               │   │
│  │ • speckit_logger_set_level()                         │   │
│  │ • speckit_logger_archive()                           │   │
│  │ • speckit_logger_flush()                             │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    C++ 实现层 (speckit::log)                 │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  ┌──────────────┐  ┌──────────────┐              │   │
│  │  │  Logger      │  │  LogManager  │              │   │
│  │  │  (核心日志器) │  │ (生命周期管理)│              │   │
│  │  └──────────────┘  └──────────────┘              │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌───────────┐ │   │
│  │  │  LogEntry    │  │  LogBuffer   │  │  Archive  │ │   │
│  │  │  (日志条目)   │  │  (环形缓冲)  │  │ (ZIP归档)│ │   │
│  │  └──────────────┘  └──────────────┘  └───────────┘ │   │
│  │  ┌──────────────┐  ┌──────────────┐                │   │
│  │  │ FileManager  │  │ CrashHandler │                │   │
│  │  │ (文件管理)   │  │ (崩溃恢复)   │                │   │
│  │  └──────────────┘  └──────────────┘                │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    异步写入层                                 │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  • Lock-Free Queue (无锁队列)                         │   │
│  │  • Background Writer Thread (后台写入线程)            │   │
│  │  • Periodic Flush Timer (定期刷新)                   │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    存储层 (本地文件系统)                       │
│  • 文本日志文件 (A.log, A.1.log, ...)                        │
│  • ZIP归档文件 (A_1234_20230116120000.zip)                   │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 架构原则

1. **分层设计**: 应用层、C-Bridge层、C++实现层、异步层、存储层
2. **职责分离**: 每个模块职责单一，高内聚低耦合
3. **异步优先**: 日志调用快速返回，后台异步写入
4. **资源预分配**: 初始化时分配所有资源，运行时避免动态内存分配
5. **崩溃安全**: 通过环形缓冲区和信号处理机制保证崩溃时数据不丢失

---

## 3. 核心模块设计

### 3.1 日志条目 (LogEntry)

**职责**: 封装单条日志的所有信息，提供格式化输出

**数据结构**:
```cpp
struct LogEntry {
    LogLevel level;              // 日志级别
    std::chrono::system_clock::time_point timestamp;  // 时间戳
    uint32_t process_id;         // 进程ID
    uint32_t thread_id;          // 线程ID
    std::string tag;             // 标签
    std::string message;         // 用户消息
    std::source_location location; // 源代码位置
    
    // 格式化为日志字符串
    std::string format() const;
};
```

**设计要点**:
- 使用 `std::string_view` 传递字符串，减少拷贝
- 时间戳使用 `std::chrono::system_clock::time_point`，精度到毫秒
- `std::source_location` 在宏展开时捕获调用位置信息
- 提供 `format()` 方法按照规范格式输出日志

### 3.2 日志缓冲区 (LogBuffer)

**职责**: 提供崩溃安全的环形缓冲区，保存未写入磁盘的日志

**数据结构**:
```cpp
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity);
    ~LogBuffer();
    
    bool push(const LogEntry& entry);
    bool pop(LogEntry& entry);
    std::vector<LogEntry> flush_all();  // 崩溃时调用
    
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    bool full() const;
    
private:
    std::unique_ptr<LogEntry[]> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    size_t capacity_;
    std::mutex mutex_;  // 用于 flush_all
};
```

**设计要点**:
- 环形缓冲区实现，预分配固定容量（例如10000条）
- 使用 `std::atomic` 实现无锁的 push/pop 操作
- `flush_all()` 方法在崩溃时被信号处理程序调用，将所有日志同步写入
- 缓冲区满时丢弃新日志，保证系统稳定性

### 3.3 文件管理器 (FileManager)

**职责**: 管理日志文件的创建、滚动、删除和重命名

**数据结构**:
```cpp
class FileManager {
public:
    FileManager(const std::string& base_name, 
                size_t max_file_size = 10 * 1024 * 1024,
                size_t max_history_files = 3);
    
    void write(const std::string& content);
    void rotate();  // 文件滚动
    
    std::vector<std::filesystem::path> get_all_log_files() const;
    void archive_to_zip(const std::string& archive_name);
    
private:
    std::string base_name_;
    std::filesystem::path current_file_path_;
    std::ofstream current_file_;
    size_t current_file_size_;
    size_t max_file_size_;
    size_t max_history_files_;
    
    void create_new_file();
    void rotate_history_files();
    std::string generate_filename(uint32_t pid = 0) const;
};
```

**设计要点**:
- 监控当前文件大小，达到 `max_file_size` 时触发滚动
- 滚动逻辑：A.log → A.1.log → A.2.log → A.3.log → (删除)
- 使用 `std::filesystem` 处理跨平台文件路径
- 提供归档接口，将所有日志文件打包成ZIP

### 3.4 归档模块 (Archive)

**职责**: 将日志文件压缩成ZIP格式

**设计要点**:
- 由于零依赖要求，使用平台原生API：
  - Windows: `CreateZipFile` / IZipArchive
  - macOS: `zip` 命令或 `zlib` (如果系统自带)
- 归档后删除原始文件
- 线程安全，支持并发调用

### 3.5 崩溃处理器 (CrashHandler)

**职责**: 注册信号处理程序，在程序崩溃时保护日志数据

**数据结构**:
```cpp
class CrashHandler {
public:
    static void install(LogBuffer* buffer);
    static void uninstall();
    
private:
    static void signal_handler(int signal);
    static LogBuffer* log_buffer_;
};
```

**设计要点**:
- 注册以下信号：SIGSEGV, SIGABRT, SIGFPE, SIGILL
- 信号处理程序中调用 `LogBuffer::flush_all()` 同步写入
- 避免在信号处理程序中使用非异步安全函数
- 使用 `std::atomic_flag` 防止重复处理

### 3.6 日志管理器 (LogManager)

**职责**: 管理日志器的生命周期和全局状态

**数据结构**:
```cpp
class LogManager {
public:
    static Logger* create_logger(const LoggerConfig& config);
    static void destroy_logger(Logger* logger);
    
    static void initialize();  // 全局初始化
    static void shutdown();    // 全局清理
    
private:
    static std::atomic<bool> initialized_;
    static std::mutex mutex_;
    static std::vector<std::unique_ptr<Logger>> loggers_;
};
```

**设计要点**:
- 单例模式管理所有日志器实例
- 提供全局初始化/清理接口
- 保证线程安全

---

## 4. 数据流设计

### 4.1 日志写入流程

```
应用调用日志接口
      │
      ▼
C-Bridge API: speckit_logger_log()
      │
      ▼
级别过滤检查 (Level < GlobalLevel?)
      │
      ├─ 是 → 直接返回（静默丢弃）
      │
      ▼ 否
构造 LogEntry 对象（包含tag信息，但不做过滤）
      │
      ▼
推入 Lock-Free Queue
      │
      ├─ 队列满 → 丢弃新日志
      │
      ▼ 队列未满
立即返回（< 1ms）
      │
      ▼
后台写入线程
      │
      ▼
从队列取出 LogEntry
      │
      ▼
格式化为字符串
      │
      ▼
推入环形缓冲区 (LogBuffer)
      │
      ▼
写入当前日志文件
      │
      ▼
检查文件大小 → 触发滚动（如需要）
```

### 4.2 崩溃恢复流程

```
程序崩溃
      │
      ▼
操作系统发送信号 (SIGSEGV/SIGABRT等)
      │
      ▼
信号处理程序 (CrashHandler)
      │
      ▼
调用 LogBuffer::flush_all()
      │
      ▼
取出所有未写入的 LogEntry
      │
      ▼
同步写入磁盘（阻塞）
      │
      ▼
关闭文件句柄
      │
      ▼
程序终止
```

### 4.3 文件滚动流程

```
写入日志到当前文件
      │
      ▼
检查文件大小
      │
      ├─ 未达限制 → 继续写入
      │
      ▼ 达到限制
重命名历史文件
  A.3.log → 删除
  A.2.log → A.3.log
  A.1.log → A.2.log
  A.log → A.1.log
      │
      ▼
创建新的 A.log
      │
      ▼
继续写入
```

### 4.4 归档流程

```
调用归档接口
      │
      ▼
列出所有日志文件
  (A.log, A.1.log, A.2.log)
      │
      ▼
创建ZIP文件
  (A_1234_20230116120000.zip)
      │
      ▼
将日志文件添加到ZIP
      │
      ▼
删除原始文件
      │
      ▼
创建新的空 A.log
      │
      ▼
归档完成
```

---

## 5. 接口设计

### 5.1 C-Bridge API 接口

#### 5.1.1 数据类型定义

```c
#ifdef __cplusplus
extern "C" {
#endif

// 日志级别枚举
typedef enum {
    SPECKIT_LOG_LEVEL_DEBUG = 0,
    SPECKIT_LOG_LEVEL_INFO = 1,
    SPECKIT_LOG_LEVEL_WARNING = 2,
    SPECKIT_LOG_LEVEL_ERROR = 3
} SpeckitLogLevel;

// 日志器句柄（不透明指针）
typedef struct SpeckitLogger SpeckitLogger;

// 配置结构体
typedef struct {
    const char* log_directory;     // 日志目录
    const char* base_name;         // 基础文件名
    SpeckitLogLevel default_level; // 默认日志级别
    size_t max_file_size;          // 最大文件大小（字节）
    size_t max_history_files;      // 最大历史文件数
    size_t buffer_capacity;        // 缓冲区容量
} SpeckitLoggerConfig;

// 错误码
typedef enum {
    SPECKIT_OK = 0,
    SPECKIT_ERROR_INVALID_ARGUMENT = -1,
    SPECKIT_ERROR_FILE_NOT_FOUND = -2,
    SPECKIT_ERROR_PERMISSION_DENIED = -3,
    SPECKIT_ERROR_DISK_FULL = -4,
    SPECKIT_ERROR_BUFFER_FULL = -5,
    SPECKIT_ERROR_UNKNOWN = -99
} SpeckitErrorCode;

#ifdef __cplusplus
}
#endif
```

#### 5.1.2 核心接口

```c
/**
 * 创建日志器实例
 * @param config 配置参数
 * @return 日志器句柄，失败返回 NULL
 */
SpeckitLogger* speckit_logger_create(const SpeckitLoggerConfig* config);

/**
 * 销毁日志器实例
 * @param logger 日志器句柄
 */
void speckit_logger_destroy(SpeckitLogger* logger);

/**
 * 记录日志
 * @param logger 日志器句柄
 * @param level 日志级别
 * @param tag 标签（可为 NULL）
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return 错误码
 */
int speckit_logger_log(SpeckitLogger* logger, 
                        SpeckitLogLevel level,
                        const char* tag,
                        const char* format, ...);

/**
 * 设置全局日志级别
 * @param logger 日志器句柄
 * @param level 日志级别
 * @return 错误码
 */
int speckit_logger_set_level(SpeckitLogger* logger, SpeckitLogLevel level);

/**
 * 手动触发日志归档
 * @param logger 日志器句柄
 * @param archive_name 归档文件名（可为 NULL，使用默认命名）
 * @return 错误码
 */
int speckit_logger_archive(SpeckitLogger* logger, const char* archive_name);

/**
 * 刷新缓冲区到磁盘
 * @param logger 日志器句柄
 * @return 错误码
 */
int speckit_logger_flush(SpeckitLogger* logger);

/**
 * 获取日志统计信息
 * @param logger 日志器句柄
 * @param total_logs 总日志数（输出）
 * @param dropped_logs 丢弃日志数（输出）
 * @param buffer_size 缓冲区当前大小（输出）
 * @return 错误码
 */
int speckit_logger_get_stats(SpeckitLogger* logger,
                              size_t* total_logs,
                              size_t* dropped_logs,
                              size_t* buffer_size);
```

### 5.2 C++ 内部接口

#### 5.2.1 Logger 类

```cpp
namespace speckit::log {

class Logger {
public:
    explicit Logger(const LoggerConfig& config);
    ~Logger();
    
    // 禁止拷贝，允许移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) noexcept;
    Logger& operator=(Logger&&) noexcept;
    
    // 日志记录接口
    void log(LogLevel level, 
             const std::string& tag,
             const std::string& message,
             const std::source_location& location = std::source_location::current());
    
    // 级别控制
    void set_level(LogLevel level);
    
    // 文件操作
    void archive(const std::string& archive_name = "");
    void flush();
    
    // 统计信息
    struct Stats {
        size_t total_logs;
        size_t dropped_logs;
        size_t buffer_size;
        size_t buffer_capacity;
    };
    Stats get_stats() const;
    
private:
    LoggerConfig config_;
    LogLevel global_level_;
    LogBuffer log_buffer_;
    FileManager file_manager_;
    std::jthread writer_thread_;
    std::atomic<bool> running_;
    
    Stats stats_;
    
    void writer_loop(std::stop_token stop_token);
    void write_entry(const LogEntry& entry);
    bool should_log(LogLevel level) const;
};

} // namespace speckit::log
```

---

## 6. 性能优化策略

### 6.1 调用延迟优化 (< 1ms)

| 优化措施 | 实现方式 | 预期效果 |
|---------|---------|---------|
| 级别过滤前置 | 在构造 LogEntry 之前检查级别，快速返回 | 减少 80% 无效调用开销 |
| 无锁队列 | 使用 `std::atomic` 实现无锁队列，避免互斥锁竞争 | 减少 60% 并发开销 |
| 字符串视图 | 使用 `std::string_view` 传递字符串，避免深拷贝 | 减少 70% 内存拷贝 |
| 资源预分配 | 初始化时分配所有内存和缓冲区，运行时零分配 | 消除动态分配开销 |
| 批量写入 | 后台线程批量写入文件，减少 I/O 系统调用 | 减少 90% I/O 开销 |

### 6.2 吞吐量优化 (100K+ logs/sec)

| 优化措施 | 实现方式 | 预期效果 |
|---------|---------|---------|
| 异步写入 | 单一后台线程集中处理写入操作 | 解耦业务逻辑和 I/O |
| 缓冲区大小 | 设置足够大的缓冲区（10000+ 条） | 吸收突发流量 |
| 文件滚动优化 | 原子操作快速切换文件 | 滚动时间 < 100ms |
| 格式化优化 | 使用 `std::format` 编译期格式字符串 | 减少 50% 格式化时间 |
| 日志池 | 使用对象池复用 LogEntry 对象 | 减少内存分配 |

### 6.3 内存优化 (< 200MB)

| 优化措施 | 实现方式 | 预期效果 |
|---------|---------|---------|
| 固定大小缓冲区 | 预分配固定容量，运行时不扩容 | 内存占用可预测 |
| 字符串池 | 复用常用字符串（标签名等） | 减少 30% 字符串内存 |
| 及时释放 | 滚动后立即关闭旧文件句柄 | 避免文件句柄泄漏 |
| 智能指针 | 使用 `std::unique_ptr` 管理资源 | 自动释放，无泄漏 |

### 6.4 崩溃安全优化 (≥ 90% 保存率)

| 优化措施 | 实现方式 | 预期效果 |
|---------|---------|---------|
| 环形缓冲区 | 所有日志先入缓冲区，再写入磁盘 | 崩溃时保留未写日志 |
| 信号处理 | 注册崩溃信号处理程序 | 触发紧急flush |
| 定期flush | 后台线程定期 flush（如每秒） | 缩小丢失窗口 |
| 原子操作 | 使用原子变量保证数据一致性 | 避免数据损坏 |

---

## 7. 异常处理机制

### 7.1 错误处理策略

| 错误类型 | 处理方式 | 用户反馈 |
|---------|---------|---------|
| 无效参数 | 返回 `SPECKIT_ERROR_INVALID_ARGUMENT`，不写入日志 | 明确错误码 |
| 文件权限不足 | 返回 `SPECKIT_ERROR_PERMISSION_DENIED`，停止写入 | 明确错误码 |
| 磁盘空间不足 | 返回 `SPECKIT_ERROR_DISK_FULL`，丢弃新日志 | 明确错误码 |
| 缓冲区满 | 返回 `SPECKIT_ERROR_BUFFER_FULL`，丢弃新日志 | 明确错误码 |
| 文件写入失败 | 重试3次，失败后返回错误 | 记录错误日志 |
| 归档失败 | 保留原始文件，返回错误 | 记录错误日志 |
| 未知错误 | 返回 `SPECKIT_ERROR_UNKNOWN` | 记录错误日志 |

### 7.2 异常场景处理

#### 7.2.1 磁盘空间不足

```cpp
void FileManager::write(const std::string& content) {
    try {
        if (current_file_.is_open()) {
            current_file_ << content << std::flush;
            current_file_size_ += content.size();
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // 检查是否为磁盘空间不足
        if (e.code() == std::errc::no_space_on_device) {
            throw std::runtime_error("Disk full");
        }
        throw;
    }
}
```

#### 7.2.2 文件权限问题

```cpp
void FileManager::create_new_file() {
    auto path = generate_filename();
    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Permission denied: " + path.string());
    }
    current_file_ = std::move(file);
    current_file_path_ = path;
    current_file_size_ = 0;
}
```

#### 7.2.3 进程崩溃

```cpp
void CrashHandler::signal_handler(int signal) {
    // 防止重复处理
    static std::atomic_flag in_handler = ATOMIC_FLAG_INIT;
    if (in_handler.test_and_set()) return;
    
    // 紧急flush缓冲区
    if (log_buffer_) {
        log_buffer_->flush_all();
    }
    
    // 恢复默认处理并重新触发信号
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}
```

### 7.3 降级策略

| 场景 | 降级措施 | 说明 |
|------|---------|------|
| 队列满 | 丢弃新日志 | 保证系统不崩溃 |
| 磁盘满 | 停止写入，记录错误 | 避免数据损坏 |
| 文件权限失败 | 切换到备用路径 | 尝试其他目录 |
| 内存不足 | 缩小缓冲区 | 降低容量继续运行 |
| 归档失败 | 保留原始文件 | 不影响日志写入 |

---

## 8. 测试策略

### 8.1 单元测试

| 测试模块 | 测试覆盖 | 工具 |
|---------|---------|------|
| LogEntry | 格式化、数据完整性 | GoogleTest |
| LogBuffer | push/pop、环形逻辑、边界条件 | GoogleTest |
| TagFilter | 启用/禁用、级别过滤、并发安全 | GoogleTest |
| FileManager | 文件创建、滚动、删除 | GoogleTest + Mock filesystem |
| CrashHandler | 信号处理、flush_all | GoogleTest + 信号注入 |

### 8.2 集成测试

| 测试场景 | 测试目标 | 工具 |
|---------|---------|------|
| 异步日志写入 | 验证后台线程正确写入 | 多线程测试程序 |
| 文件滚动 | 验证滚动逻辑和原子性 | 填充日志文件 |
| 多进程隔离 | 验证进程间日志不干扰 | 启动多个进程 |
| 日志归档 | 验证ZIP打包和文件删除 | 归档接口调用 |
| 崩溃恢复 | 验证崩溃时数据保存 | 触发崩溃信号 |

### 8.3 性能测试

| 测试指标 | 测试方法 | 目标值 |
|---------|---------|--------|
| 调用延迟 | 测量 `log()` 函数耗时 | < 1ms |
| 吞吐量 | 持续写入日志，测量条/秒 | > 100K 条/秒 |
| 并发性能 | 多线程并发写入，测量性能下降 | < 20% 下降 |
| 内存占用 | 长时间运行，监控内存 | < 200MB |
| 文件滚动 | 测量滚动操作耗时 | < 100ms |
| 归档性能 | 归档 10MB 日志 | < 5s |

### 8.4 压力测试

| 测试场景 | 测试方法 | 预期结果 |
|---------|---------|---------|
| 高频写入 | 100K+ 条/秒持续写入 | 不丢失、不重复 |
| 长时间运行 | 24小时连续运行 | 无内存泄漏、性能稳定 |
| 极端并发 | 50+ 线程同时写入 | 系统稳定 |
| 磁盘满 | 模拟磁盘空间耗尽 | 优雅降级，不崩溃 |
| 快速崩溃 | 循环触发崩溃信号 | 90%+ 数据保存 |

---

## 9. 部署与运维

### 9.1 编译构建

#### Windows

```bash
# 安装依赖
# Visual Studio 2022 with C++20 support

# 构建
cd AppSimpleLog
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### macOS

```bash
# 安装依赖
# Xcode Command Line Tools (includes C++20 support)

# 构建
cd AppSimpleLog
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

### 9.2 集成方式

#### C++ 项目

```cpp
#include "speckit/log/speckit_logger.h"

SpeckitLoggerConfig config = {
    .log_directory = "./logs",
    .base_name = "app",
    .default_level = SPECKIT_LOG_LEVEL_INFO,
    .max_file_size = 10 * 1024 * 1024,
    .max_history_files = 3,
    .buffer_capacity = 10000
};

auto* logger = speckit_logger_create(&config);
speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Network", 
                   "Connection established");
speckit_logger_destroy(logger);
```

#### Objective-C 项目

```objective-c
#import "speckit_log.h"

SpeckitLoggerConfig config;
config.log_directory = [logsPath UTF8String];
config.base_name = "app";
config.default_level = SPECKIT_LOG_LEVEL_INFO;

SpeckitLogger* logger = speckit_logger_create(&config);
speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "UI", 
                   "Button clicked");
speckit_logger_destroy(logger);
```

### 9.3 日志目录结构

```
logs/
├── app.log                    # 主进程日志
├── app_1234.log               # 进程 1234 的日志
├── app.1.log                  # 历史文件 1
├── app.2.log                  # 历史文件 2
├── app.3.log                  # 历史文件 3
└── app_1234_20230116120000.zip # 归档文件
```

### 9.4 监控与诊断

| 监控指标 | 收集方式 | 告警阈值 |
|---------|---------|---------|
| 日志丢弃率 | `speckit_logger_get_stats()` | > 1% |
| 缓冲区使用率 | `stats.buffer_size / stats.buffer_capacity` | > 80% |
| 磁盘空间 | 文件系统检查 | < 1GB |
| 文件滚动频率 | 监控文件创建 | > 10次/小时 |

---

## 10. 风险评估与缓解

### 10.1 技术风险

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|---------|
| C++20 std::format 不支持 | 高 | 低 | 使用第三方库（如 fmt）作为备选 |
| 跨平台文件系统差异 | 中 | 中 | 充分测试，使用 std::filesystem 抽象 |
| 零依赖实现复杂度 | 高 | 中 | 详细设计，模块化开发 |
| 崩溃恢复效果 | 高 | 中 | 充分测试多种崩溃场景 |
| 性能不达标 | 中 | 低 | 性能测试驱动开发 |

### 10.2 运维风险

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|---------|
| 日志文件占用磁盘过大 | 中 | 中 | 实现自动滚动和归档 |
| 多进程日志冲突 | 高 | 低 | 进程ID隔离设计 |
| 日志丢失 | 高 | 低 | 崩溃安全机制 + 定期flush |
| 文件权限问题 | 中 | 低 | 提供权限检查和错误提示 |

### 10.3 不确定事项

以下事项需要进一步确认：

1. **归档方案**: 由于零依赖限制，需要确认目标平台是否提供原生 ZIP API，否则需要实现简单压缩算法
2. **时间戳格式**: 需要确认是否使用本地时区还是 UTC，以及跨时区部署的处理方式
3. **崩溃恢复窗口**: 定期flush的间隔（如1秒、5秒）需要根据实际场景权衡
4. **缓冲区容量**: 默认缓冲区大小需要根据日志量级调整

---

## 11. 接口图

### 11.1 模块依赖关系图

```
┌──────────────────────────────────────────────────────────────┐
│                        Application                            │
│                   (C++ / Objective-C)                         │
└──────────────────────────────────────────────────────────────┘
                            │
                            │ calls
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                      C-Bridge API                             │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐            │
│  │ speckit_*  │  │ Speckit*   │  │  Config    │            │
│  │ functions  │  │  enums     │  │  structs   │            │
│  └────────────┘  └────────────┘  └────────────┘            │
└──────────────────────────────────────────────────────────────┘
                            │
                            │ implements
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                    C++ Core Layer                            │
│  ┌──────────────────────────────────────────────────────┐    │
│  │                     Logger                           │    │
│  │  - log()                                            │    │
│  │  - set_level()                                      │    │
│  │  - archive()                                        │    │
│  └──────────────┬───────────────────────────────────────┘    │
│                 │                                             │
│      ┌──────────┼──────────┬──────────┐                      │
│      ▼          ▼          ▼          ▼                      │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐            │
│  │LogEntry │ │TagFilter│ │LogBuffer│ │FileManager│           │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘            │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐    │
│  │                    LogManager                        │    │
│  │  - create_logger()                                   │    │
│  │  - destroy_logger()                                 │    │
│  └──────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐    │
│  │                   CrashHandler                       │    │
│  │  - signal_handler()                                   │    │
│  └──────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
                            │
                            │ uses
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                 C++20 Standard Library                       │
│  - std::format, std::source_location                         │
│  - std::atomic, std::jthread                                 │
│  - std::filesystem, std::chrono                              │
│  - std::mutex, std::condition_variable                       │
└──────────────────────────────────────────────────────────────┘
                            │
                            │ writes to
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                   File System Storage                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ Text Log     │  │ Rotated      │  │ ZIP Archive  │       │
│  │ Files        │  │ Files        │  │              │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
└──────────────────────────────────────────────────────────────┘
```

### 11.2 日志写入流程图

```
┌────────────────┐
│ App calls      │
│ log() function │
└────────┬───────┘
         │
         ▼
┌──────────────────────────────────┐
│ speckit_logger_log()             │
│ (C-Bridge API)                   │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Level < GlobalLevel?             │
└────────┬─────────────────────────┘
         │
    ┌────┴────┐
    │ Yes     │ No
    ▼         ▼
┌─────────┐ ┌──────────────────────────┐
│ Return  │ │ Create LogEntry         │
│ (drop)  │ │ - level                 │
└─────────┘ │ - timestamp             │
           │ - pid, tid              │
           │ - tag (透传)             │
           │ - message                │
           │ - source_location       │
           └────────┬─────────────────┘
                              │
                              ▼
                     ┌──────────────────────────┐
                     │ Push to Lock-Free Queue  │
                     └────────┬─────────────────┘
                              │
                        ┌─────┴─────┐
                        │ Full?     │ Not Full
                        ▼           ▼
                   ┌─────────┐ ┌───────────────┐
                   │ Return  │ │ Return        │
                   │ (drop)  │ │ (< 1ms)       │
                   └─────────┘ └───────┬───────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ Background       │
                             │ Writer Thread    │
                             └────────┬─────────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ Pop from Queue   │
                             └────────┬─────────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ Push to         │
                             │ LogBuffer       │
                             └────────┬─────────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ Format to String │
                             └────────┬─────────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ Write to File   │
                             └────────┬─────────┘
                                      │
                                      ▼
                             ┌──────────────────┐
                             │ File Size > Max? │
                             └────────┬─────────┘
                                      │
                                ┌─────┴─────┐
                                │ Yes       │ No
                                ▼           │
                        ┌───────────┐        │
                        │ Rotate    │        │
                        │ Files     │        │
                        └───────────┘        │
                                      ▼
                             ┌──────────────────┐
                             │ Periodic Flush   │
                             │ (e.g., every 1s) │
                             └──────────────────┘
```

### 11.3 文件滚动流程图

```
┌──────────────────────────────────┐
│ Writing to current file         │
│ (A.log)                         │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Current file size >= Max Size?   │
└────────┬─────────────────────────┘
         │
    ┌────┴────┐
    │ No      │ Yes
    ▼         ▼
┌─────────┐ ┌──────────────────────────┐
│ Continue│ │ Close current file      │
│ Writing │ └────────┬─────────────────┘
└─────────┘          │
                     ▼
         ┌──────────────────────────────────┐
         │ Rotate history files:            │
         │ A.3.log → Delete                 │
         │ A.2.log → A.3.log                │
         │ A.1.log → A.2.log                │
         │ A.log → A.1.log                  │
         └────────┬─────────────────────────┘
                  │
                  ▼
         ┌──────────────────────────────────┐
         │ Create new A.log                 │
         └────────┬─────────────────────────┘
                  │
                  ▼
         ┌──────────────────────────────────┐
         │ Continue writing                 │
         └──────────────────────────────────┘
```

### 11.4 崩溃恢复流程图

```
┌──────────────────────────────────┐
│ Application Crash               │
│ (SIGSEGV / SIGABRT / etc.)      │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Signal Triggered                 │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ CrashHandler::signal_handler()   │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Check atomic_flag (in_handler)   │
└────────┬─────────────────────────┘
         │
    ┌────┴────┐
    │ Set     │ Already Set
    ▼         ▼
┌───────────────────┐ ┌─────────┐
│ LogBuffer::       │ │ Return  │
│   flush_all()     │ │         │
└────────┬──────────┘ └─────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Extract all LogEntry from buffer │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ For each entry:                  │
│   - Format to string             │
│   - Write to disk (sync)         │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Close file handles               │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Restore default signal handler   │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Re-raise signal                 │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Process terminates               │
└──────────────────────────────────┘
```

### 11.5 归档流程图

```
┌──────────────────────────────────┐
│ User calls                       │
│ speckit_logger_archive()         │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Generate archive name            │
│ (base_pid_timestamp.zip)         │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ List all log files               │
│ (A.log, A.1.log, A.2.log, ...)   │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Create empty ZIP file            │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ For each log file:               │
│   - Add file to ZIP              │
│   - Compress                     │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Close ZIP file                   │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Delete original log files        │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Create new empty A.log           │
└────────┬─────────────────────────┘
         │
         ▼
┌──────────────────────────────────┐
│ Return success                    │
└──────────────────────────────────┘
```

### 11.6 状态转换图

```
                    ┌───────────┐
                    │  CREATED  │
                    └─────┬─────┘
                          │ speckit_logger_create()
                          ▼
                   ┌──────────────┐
                   │ INITIALIZING │
                   └──────┬───────┘
                          │
                          ▼
                   ┌──────────────┐
                   │   RUNNING    │◄─────┐
                   └──────┬───────┘      │
                          │              │
         ┌─────────────────┼──────────────┤
         │                 │              │
         │ flush()         │ archive()    │ crash
         ▼                 ▼              ▼
  ┌─────────────┐   ┌─────────────┐  ┌──────────┐
  │  FLUSHING   │   │  ARCHIVING  │  │ CRASHED  │
  └──────┬──────┘   └──────┬──────┘  └────┬─────┘
         │                │              │
         └─────────────────┴──────────────┤
                          │               │
                          ▼               ▼
                   ┌──────────────┐  ┌─────────────┐
                   │   DESTROYED  │  │  EMERGENCY   │
                   └──────────────┘  │  FLUSH       │
                                    └──────┬───────┘
                                           │
                                           ▼
                                    ┌─────────────┐
                                    │   DESTROYED │
                                    └─────────────┘
```

---

## 12. 附录

### 12.1 术语表

| 术语 | 定义 |
|------|------|
| C-Bridge | C语言桥接层，用于跨语言调用 |
| Lock-Free Queue | 无锁队列，基于原子操作实现 |
| Ring Buffer | 环形缓冲区，固定容量循环使用 |
| Log Rotation | 日志滚动，文件达到大小限制时创建新文件 |
| Tag Filter | 标签过滤器，按标签控制日志输出 |
| Crash-Safe | 崩溃安全，程序崩溃时数据不丢失 |
| Signal Handler | 信号处理程序，捕获系统信号 |

### 12.2 参考资料

- C++20 Standard Library Documentation
- GoogleTest User Guide
- CMake Documentation
- POSIX Signal Handling
- Windows Exception Handling

### 12.3 变更记录

| 版本 | 日期 | 作者 | 变更内容 |
|------|------|------|---------|
| v1.1 | 2026-01-16 | AI | 移除标签过滤功能，仅保留标签透传 |
| v1.0 | 2026-01-16 | AI | 初始版本 |

---

**文档结束**
