# 技术方案说明书：日志查看器 (SimpleLogView)

## 1. 项目概述

### 1.1 项目名称
日志查看器 (SimpleLogView)

### 1.2 项目描述
基于MVP架构模式构建的高性能日志查看器，专为处理大型日志文件而设计。该工具实现了多种设计模式，包括观察者模式、策略模式、组合模式、中介者模式和备忘录模式，以确保模块间的松耦合和系统的可扩展性。

### 1.3 核心功能
- 基本日志查看 (支持大型文件)
- 日志级别和标签过滤
- 正则表达式搜索和高亮
- 表格交互和数据复制
- 显示自定义和窗口管理

## 2. 架构设计

### 2.1 MVP架构模式
采用Model-View-Presenter (MVP) 架构模式，实现关注点分离：

- **Model层**: 负责数据管理和业务逻辑，包括日志解析、过滤和元数据提取
- **View层**: 负责用户界面展示和用户输入捕获
- **Presenter层**: 作为Model和View之间的中介，处理它们的交互

### 2.2 应用的设计模式

#### 2.2.1 观察者模式 (Observer Pattern)
- **应用场景**: 文件加载通知 (Model → Presenter → View)
- **实现**: Model层通过信号槽机制通知Presenter和View数据变化

#### 2.2.2 策略模式 (Strategy Pattern)
- **应用场景**: 可插拔搜索算法 (正则表达式 vs. 简单文本)
- **实现**: 不同的过滤策略可以动态切换

#### 2.2.3 组合模式 (Composite Pattern)
- **应用场景**: 过滤条件组合 (级别 AND 标签 AND 正则)
- **实现**: 支持多个过滤条件的组合操作

#### 2.2.4 中介者模式 (Mediator Pattern)
- **应用场景**: Presenter协调Model和View
- **实现**: Presenter作为Model和View之间的中介，处理它们的交互

#### 2.2.5 备忘录模式 (Memento Pattern)
- **应用场景**: 窗口状态保存/恢复
- **实现**: 保存和恢复窗口大小、位置、颜色方案等设置

## 3. 接口定义

### 3.1 Model层接口 (ILogModel.h)

#### 3.1.1 ILogModel
```cpp
/**
 * @brief 日志数据模型接口 (观察者模式主题)
 * @pattern Observer
 * @role ConcreteSubject
 */
class ILogModel : public QObject {
    Q_OBJECT

public:
    virtual ~ILogModel() = default;

    // 数据访问 (只读从View/Presenter)
    virtual size_t entryCount() const = 0;
    virtual const LogEntry& entryAt(size_t index) const = 0;
    virtual bool canFetchMore() const = 0;
    virtual void fetchMore(size_t count = 100) = 0;

    // 过滤 (只写从Presenter)
    virtual size_t applyFilter(const LogFilterCriteria& filter) = 0;
    virtual void clearFilter() = 0;
    virtual size_t filteredCount() const = 0;

    // 文件操作 (只写从Presenter)
    virtual bool loadFile(const QString& filePath) = 0;

    // 元数据 (只读从Presenter/View)
    virtual std::unordered_set<std::string> detectedLevels() const = 0;
    virtual std::unordered_set<std::string> detectedTags() const = 0;
    virtual ParseStatistics statistics() const = 0;

signals:
    void dataChanged();
    void filterApplied(size_t matchCount);
    void fileLoaded(bool success, const QString& error);
    void loadingProgress(int linesLoaded, int totalLines);
};
```

#### 3.1.2 ILogParser
```cpp
/**
 * @brief 日志行解析接口 (策略模式)
 * @pattern Strategy
 * @role ConcreteStrategy
 */
class ILogParser {
public:
    virtual ~ILogParser() = default;

    virtual std::optional<LogEntry> parseLine(std::string_view line) const = 0;
    virtual bool isValidFormat(std::string_view line) const = 0;
    virtual std::string parserName() const = 0;
};
```

#### 3.1.3 IFilterStrategy
```cpp
/**
 * @brief 日志条目过滤接口 (策略模式)
 * @pattern Strategy
 * @role ConcreteStrategy
 */
class IFilterStrategy {
public:
    virtual ~IFilterStrategy() = default;

    virtual bool matches(const LogEntry& entry) const = 0;
    virtual QString description() const = 0;
    virtual bool isValid() const = 0;
};
```

### 3.2 Presenter层接口 (IPresenter.h)

#### 3.2.1 IMainPresenter
```cpp
/**
 * @brief 主Presenter接口 (MVP中介层)
 * @pattern Mediator
 * @role ConcreteMediator
 */
class IMainPresenter : public QObject {
    Q_OBJECT

public:
    virtual ~IMainPresenter() = default;

    // 初始化 (从Application)
    virtual void initialize(
        ILogModel* model,
        IMainView* view,
        ISettingsPersistence* settings
    ) = 0;

    // 文件操作 (从View)
    virtual void openFile(const QString& filePath) = 0;
    virtual void openFileDialog() = 0;

    // 过滤操作 (从View)
    virtual void onLevelFilterChanged(const std::vector<std::string>& levels) = 0;
    virtual void onTagFilterChanged(const std::vector<std::string>& tags) = 0;
    virtual void onRegexFilterRequested(const QString& pattern) = 0;

    // 复制操作 (从View)
    virtual QString getSelectedTextForCopy() = 0;
    virtual void onRowSelected(int row) = 0;

    // 设置操作 (从View)
    virtual void onDisplaySettingsRequested() = 0;
    virtual void onColorSchemeChanged(const ColorScheme& scheme) = 0;

    // 状态查询 (用于测试/验证)
    virtual QString currentFilePath() const = 0;
    virtual LogFilterCriteria currentFilter() const = 0;
    virtual ColorScheme currentColorScheme() const = 0;
    virtual bool isProcessing() const = 0;
    virtual bool hasFileLoaded() const = 0;
};
```

### 3.3 View层接口 (IView.h)

#### 3.3.1 IMainView
```cpp
/**
 * @brief 主应用视图接口 (MVP视图层)
 * @pattern Observer
 * @role ConcreteObserver
 */
class IMainView : public QWidget {
    Q_OBJECT

public:
    virtual ~IMainView() = default;

    // 工具栏动作 (信号)
    void openFileRequested();
    void levelFilterChanged(const std::vector<std::string>& levels);
    void tagFilterChanged(const std::vector<std::string>& tags);
    void regexFilterRequested(const QString& pattern);
    void displaySettingsRequested();

    // 表格动作 (信号)
    void copyRequested();
    void rowSelected(int row);

    // 更新槽 (从Presenter)
    virtual void updateData(size_t entryCount) = 0;
    virtual void applyColorScheme(const ColorScheme& scheme) = 0;
    virtual void showError(const QString& message) = 0;
    virtual void showLoadingProgress(int current, int total) = 0;
    virtual void hideLoadingProgress() = 0;
    virtual void showStatus(const QString& message) = 0;

    // 配置 (从Presenter)
    virtual void setControlsEnabled(bool enabled) = 0;
    virtual void setWindowTitle(const QString& title) = 0;
};
```

#### 3.3.2 ILogTableView
```cpp
/**
 * @brief 日志表视图接口 (复合UI组件)
 * @pattern Composite
 * @role LeafComponent
 */
class ILogTableView : public QWidget {
    Q_OBJECT

public:
    virtual ~ILogTableView() = default;

    // 数据更新 (从Presenter)
    virtual void setModel(QAbstractItemModel* model) = 0;

    // 显示更新 (从Presenter)
    virtual void applyColorScheme(const ColorScheme& scheme) = 0;
    virtual void updateRowHighlight(int row, const std::vector<HighlightMatch>& highlights) = 0;
    virtual void updateAllHighlights(const std::vector<std::vector<HighlightMatch>>& highlights) = 0;

    // 选择 (信号)
    void copySelectionRequested();
    void rowSelected(int row);
};
```

## 4. 数据模型

### 4.1 LogEntry
表示单个日志条目的结构体：
```cpp
struct LogEntry {
    int line_number;
    std::string level;          // 日志级别
    std::string timestamp;     // 时间戳
    int process_id;            // 进程ID
    int thread_id;             // 线程ID
    std::string tag;           // 标签
    std::string content;       // 内容
    std::string raw_line;     // 原始行
    std::vector<HighlightMatch> highlights; // 高亮匹配项
};
```

### 4.2 LogFilterCriteria
定义日志条目的过滤条件：
```cpp
class LogFilterCriteria {
public:
    std::vector<std::string> selected_levels;
    bool level_filter_enabled;
    std::vector<std::string> selected_tags;
    bool tag_filter_enabled;
    QString regex_pattern;
    bool regex_filter_enabled;
    bool regex_case_sensitive;
    QColor regex_highlight_color;
    
    enum class CombinationLogic {
        AND,   // 所有筛选条件必须匹配
        OR     // 任一筛选条件匹配即可
    };
    CombinationLogic logic;
};
```

### 4.3 ColorScheme
存储用户定义的颜色映射：
```cpp
class ColorScheme {
public:
    static constexpr QColor DEFAULT_INFO_COLOR = QColor(0, 128, 0);      // 绿色
    static constexpr QColor DEFAULT_DEBUG_COLOR = QColor(100, 100, 100);   // 灰色
    static constexpr QColor DEFAULT_WARNING_COLOR = QColor(255, 165, 0);  // 橙色
    static constexpr QColor DEFAULT_ERROR_COLOR = QColor(255, 0, 0);     // 红色
    
    // 用户自定义颜色映射
    std::map<std::string, QColor> level_colors;
    QColor regex_highlight_color;
};
```

## 5. 性能要求

- **文件加载**: 5MB文件在≤3秒内加载
- **内存使用**: ≤文件大小的1.5倍
- **UI响应**: ≤100毫秒
- **搜索响应**: ≤500毫秒
- **过滤响应**: ≤500毫秒
- **文件大小限制**: 最大支持70MB文件
- **加载进度更新频率**: Loading对话框最小更新频率为1秒

## 6. 实现计划

### 6.1 增量交付策略

1. **增量1.0 (第1-2周)**: 基础日志查看器 - MVP模式基础
   - 模式: 观察者模式、组合模式、中介者模式
   - 重点: 文件加载、基本显示

2. **增量2.0 (第3-4周)**: 搜索功能 - 策略模式应用
   - 模式: 策略模式、命令模式
   - 重点: 正则表达式搜索、高亮

3. **增量3.0 (第5-6周)**: 过滤系统 - 组合模式应用
   - 模式: 组合模式、装饰器模式
   - 重点: 多条件过滤

4. **增量4.0 (第7-8周)**: 显示定制 - 备忘录模式应用
   - 模式: 备忘录模式、观察者模式
   - 重点: 布局、主题、设置持久化

## 7. 特殊功能需求

### 7.1 内存管理策略
- **超大文件处理**: 对于大于50MB的文件，实现流式处理或分页加载机制
- **懒加载**: 使用`fetchMore()`方法实现数据的逐步加载
- **内存限制**: 实施智能缓存策略，当内存接近限制时自动释放旧数据

### 7.2 多线程处理
- **后台线程**: 文件加载和过滤操作在后台线程中执行，避免UI冻结
- **加载进度指示器**: 加载过程中弹出loading对话框，展示加载进度
- **线程安全**: 确保Model层的数据访问是线程安全的

### 7.3 文件格式兼容性
- **标准格式支持**: 仅支持标准格式 `[LEVEL]TIME[PID,TID] tag:TAG MESSAGE`
- **编码**: UTF-8编码，支持中文和英文字符

### 7.4 过滤功能需求
- **筛选逻辑**: 仅保留AND（所有条件必须匹配），无需OR
- **筛选条件持久化**: 无需筛选条件持久化，启动时使用默认设置

### 7.5 其他功能需求
- **实时日志监控**: 不需要支持"tail -f"功能
- **导出功能**: 无需添加将过滤结果导出为不同格式的功能
- **国际化**: 仅需支持中文和英文
- **安全性**: 无需对加载的文件进行安全检查

## 8. 异常情况处理

### 8.1 边界情况
- 当日志文件超过70MB时，启用流式处理或分页加载
- 如何处理具有不一致行结尾的日志文件 (Windows vs Unix)
- 当日志文件具有重复时间戳或缺失字段时如何处理
- 当过滤组合导致零匹配时如何处理
- 如何处理超出列宽的极长日志行

### 8.2 错误处理
- 文件加载错误：显示错误对话框并记录日志
- 无效正则表达式：提供语法错误提示
- 内存不足：限制加载的条目数量
- 解析错误：跳过无效行并继续处理有效条目

## 9. 审查要点

以下是我认为可能需要进一步确认的问题：

1. **超大文件处理策略**：是否需要预设一个最大文件大小限制？
2. **性能基准测试**：虽然暂时没有具体的性能基准测试计划，但是否需要建立基本的性能监控指标？
3. **用户体验**：对于超大文件的加载，loading对话框的最小更新频率是多少？是否需要提供取消加载的选项？

---
**文档版本**: 1.1  
**最后更新**: 2026-01-19  
**作者**: SimpleLogView团队