# Data Model: Log Viewer with MVP Architecture

**Feature**: 1-log-viewer
**Generated**: 2026-01-19
**Input**: Entity extraction from [spec.md](spec.md)

---

## Overview

This document defines the data model entities for SimpleLogView application, following MVP architecture principles and constitution requirements for design pattern application.

**Design Patterns Applied:**
- **Observer Pattern**: Model notifies Presenter of data changes
- **Strategy Pattern**: Pluggable filtering and search algorithms
- **Composite Pattern**: Multi-condition filter combinations
- **Memento Pattern**: Window state persistence

---

## Core Entities

### 1. LogEntry

**Purpose**: Represents a single parsed log line with all metadata fields.

**Pattern Role**: Data entity (Model layer)

```cpp
/**
 * @brief Represents a single log entry with parsed fields
 *
 * @details Contains all metadata extracted from a log line:
 * - Log level (info, debug, warning, error, etc.)
 * - Timestamp for chronological ordering
 * - Process ID and thread ID for source identification
 * - Tag for component categorization
 * - Raw content for display and searching
 *
 * @pattern None (Data entity)
 * @role ConcreteSubject (Observer pattern participant)
 *
 * @note Follows Google C++ Style Guide naming conventions
 * @note Uses std::string for storage, QString for Qt integration
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
struct LogEntry {
    // Core fields (from log format: [LEVEL]TIME[PID,TID] tag:TAG MESSAGE)
    int line_number;
    std::string level;          // Case-insensitive, e.g., "info", "debug", "warning", "error"
    std::string timestamp;     // ISO 8601 format or similar
    int process_id;            // Process identifier from [PID,TID]
    int thread_id;             // Thread identifier from [PID,TID]
    std::string tag;           // Extracted from "tag:" prefix
    std::string content;       // Full message content after tag

    // Raw data (for copy functionality)
    std::string raw_line;     // Original line for clipboard copy

    // Highlighting metadata (for regex matches)
    std::vector<HighlightMatch> highlights;

    // Constructor
    LogEntry(
        int ln,
        std::string_view lvl,
        std::string_view ts,
        int pid,
        int tid,
        std::string_view tg,
        std::string_view msg,
        std::string_view raw
    );
};

/**
 * @brief Represents a regex match highlight position
 *
 * @details Used for highlighting matched text in log entries
 *
 * @pattern None (Data entity)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
struct HighlightMatch {
    size_t start_pos;
    size_t end_pos;
    QColor color;              // Qt color for highlighting
};
```

**Validation Rules:**
- `line_number` must be positive (≥ 0)
- `level` must be non-empty, converted to lowercase for case-insensitive comparison
- `process_id` and `thread_id` must be non-negative (≥ 0)
- `raw_line` must match original file content exactly

**State Transitions:**
- Created from parser → immutable after creation (no modifications to parsed fields)
- Added to Model → immutable (recreate Model for filtering changes)

---

### 2. LogFilterCriteria

**Purpose**: Defines filter conditions for log entries, supporting multiple filter types.

**Pattern Role**: Strategy pattern participant (ConcreteStrategy for filtering logic)

```cpp
/**
 * @brief Defines filter criteria for log entries
 *
 * @details Supports level filtering, tag filtering, and regex pattern matching.
 * Filters can be combined using Composite pattern for AND/OR logic.
 *
 * @pattern Strategy (Design Pattern)
 * @role ConcreteStrategy (Strategy pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class LogFilterCriteria {
public:
    // Level filtering
    std::vector<std::string> selected_levels;  // All levels to show (empty = all)
    bool level_filter_enabled;

    // Tag filtering
    std::vector<std::string> selected_tags;    // All tags to show (empty = all)
    bool tag_filter_enabled;

    // Regex filtering
    QString regex_pattern;
    bool regex_filter_enabled;
    bool regex_case_sensitive;
    QColor regex_highlight_color;

    // Combination logic
    enum class CombinationLogic {
        AND,   // All filters must match (default)
        OR      // Any filter must match
    };
    CombinationLogic logic;

    // Constructor
    LogFilterCriteria();

    // Validation
    bool isValid() const;
    QString validationError() const;

    // Matching interface
    bool matches(const LogEntry& entry) const;

    // Operator overloads for composite pattern
    bool operator==(const LogFilterCriteria& other) const;
};
```

**Validation Rules:**
- `selected_levels` can be empty (show all) or contain lowercase level names
- `selected_tags` can be empty (show all) or contain exact tag strings
- `regex_pattern` must be valid PCRE when `regex_filter_enabled` is true
- `logic` defaults to `AND` (all active filters must match)

**State Transitions:**
- Created → Modified → Destroyed
- Changes trigger Model re-filtering and Presenter notification

---

### 3. ColorScheme

**Purpose**: Stores user-defined color mappings for log levels and regex highlights.

**Pattern Role**: Memento participant (part of window state persistence)

```cpp
/**
 * @brief Color configuration for log display
 *
 * @details Maps log levels and UI elements to Qt QColor values.
 * Supports custom user-defined colors with defaults.
 *
 * @pattern Memento (Design Pattern)
 * @role ConcreteMemento (Memento pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ColorScheme {
public:
    // Default colors (per constitution requirements)
    static constexpr QColor DEFAULT_INFO_COLOR = QColor(0, 128, 0);      // Green
    static constexpr QColor DEFAULT_DEBUG_COLOR = QColor(100, 100, 100);   // Gray
    static constexpr QColor DEFAULT_WARNING_COLOR = QColor(255, 165, 0);  // Orange
    static constexpr QColor DEFAULT_ERROR_COLOR = QColor(220, 20, 60);     // Red
    static constexpr QColor DEFAULT_REGEX_HIGHLIGHT = QColor(255, 255, 0); // Yellow

    // Log level colors
    QColor info_color;
    QColor debug_color;
    QColor warning_color;
    QColor error_color;
    QColor custom_level_color;  // For user-defined levels

    // Regex highlighting
    QColor regex_match_color;

    // UI colors
    QColor background_color;
    QColor text_color;
    QColor selection_color;

    // Constructor
    ColorScheme();

    // Serialization (for QSettings)
    QByteArray serialize() const;
    static ColorScheme deserialize(const QByteArray& data);

    // Reset to defaults
    void resetToDefaults();

    // Validation
    bool isValid() const;
};
```

**Validation Rules:**
- All colors must be valid (QColor::isValid())
- No color can be fully transparent (alpha channel > 0)
- RGB values must be in range [0-255]

**State Transitions:**
- Created → Modified (by user) → Reset to defaults → Destroyed

---

### 4. WindowSettings

**Purpose**: Stores window geometry, column configurations, and UI state for persistence.

**Pattern Role**: Memento participant (captures window state for restore)

```cpp
/**
 * @brief Window geometry and UI state persistence
 *
 * @details Stores window size, position, column widths, and layout state
 * for restoration between application sessions.
 *
 * @pattern Memento (Design Pattern)
 * @role ConcreteMemento (Memento pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class WindowSettings {
public:
    // Window geometry
    QByteArray window_geometry;      // From QWidget::saveGeometry()
    QByteArray window_state;        // From QWidget::saveState()

    // Column configurations (for 6-column table)
    struct ColumnConfig {
        int column_index;
        int width;
        bool visible;
        int sort_order;          // -1 = no sort, 0 = asc, 1 = desc
    };
    std::vector<ColumnConfig> columns;

    // Splitter state (for draggable panels)
    QByteArray splitter_state;

    // Constructor
    WindowSettings();

    // Serialization (for QSettings)
    QByteArray serialize() const;
    static WindowSettings deserialize(const QByteArray& data);

    // Validation
    bool isValid() const;
};
```

**Validation Rules:**
- `window_geometry` must be non-empty and valid QByteArray
- `columns` must contain exactly 6 configurations (one per table column)
- Each `column.width` must be positive (> 0 pixels)
- `column.visible` must be boolean
- `column.sort_order` must be in range [-1, 1]

**State Transitions:**
- Created → Updated (on window resize/column change) → Saved → Loaded (on app start)

---

### 5. ParsedLogData

**Purpose**: Collection of LogEntry objects with metadata about source file and parsing statistics.

**Pattern Role**: Subject in Observer pattern (notifies observers of changes)

```cpp
/**
 * @brief Container for parsed log data with metadata
 *
 * @details Maintains collection of LogEntry objects, file metadata,
 * and statistics about parsing success/failure rates.
 *
 * @pattern Observer (Design Pattern)
 * @role ConcreteSubject (Observer pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ParsedLogData : public QObject {
    Q_OBJECT

public:
    // Data access
    size_t entryCount() const;
    const LogEntry& entryAt(size_t index) const;

    // Metadata
    std::string file_path;
    std::uintmax_t file_size;
    size_t total_lines_parsed;
    size_t valid_entries;
    size_t malformed_lines;      // Skipped per spec requirements
    std::uintmax_t parse_time_ms;

    // Filtering (live data)
    std::vector<size_t> filtered_indices;  // Indices of entries matching current filter

    // Statistics (auto-extracted from file)
    std::vector<std::string> detected_levels;
    std::vector<std::string> detected_tags;
    std::unordered_map<std::string, size_t> level_counts;
    std::unordered_map<std::string, size_t> tag_counts;

    // Constructor
    ParsedLogData(QObject* parent = nullptr);

    // Data operations
    void addEntry(const LogEntry& entry);
    void clear();
    void applyFilter(const LogFilterCriteria& filter);

    // Signals (Observer pattern)
signals:
    void dataChanged();                     // Emitted when entries are added or cleared
    void filterApplied(size_t match_count); // Emitted after filtering
    void fileLoaded(bool success, const QString& error); // Emitted after file load

private:
    std::vector<LogEntry> entries_;
    std::mutex data_mutex_;  // Thread safety for background loading
};
```

**Validation Rules:**
- `entryCount()` must equal `entries_.size()`
- `valid_entries + malformed_lines` must equal `total_lines_parsed`
- `file_size` must match actual file on disk (if file accessible)
- All `filtered_indices` must be within [0, entryCount())

**State Transitions:**
- Empty → Loading → Loaded → Filtered → Cleared → Destroyed

---

## Entity Relationships

### Dependency Graph

```
WindowSettings (Memento)
    ↓ saved to/loaded from
ColorScheme (Memento)

    ↓ applied to
LogEntry (Data Entity)
    ↓ contained in
ParsedLogData (Subject/Observer)
    ↓ observed by
MainPresenter (Mediator)

    ↓ uses
LogFilterCriteria (Strategy)
```

### Lifecycle

1. **LogEntry**: Created during parsing, immutable, destroyed when Model is destroyed
2. **LogFilterCriteria**: Created by user interaction, modified on filter changes, destroyed when filter is reset
3. **ColorScheme**: Loaded from settings on app start, modified by user, saved to settings on change
4. **WindowSettings**: Loaded from settings on app start, modified by window actions, saved to settings on close
5. **ParsedLogData**: Created on file load, updated on filter changes, destroyed on Model destruction

---

## Data Access Patterns

### Observer Pattern Implementation

**Subject**: `ParsedLogData` (maintains log entries and notifies changes)

**Observer**: `MainPresenter` (receives notifications and updates View)

**Signal/Slot Connections:**
```cpp
// In Presenter (connects to Model signals)
connect(model, &ParsedLogData::dataChanged,
        this, &MainPresenter::onDataChanged);

connect(model, &ParsedLogData::filterApplied,
        this, &MainPresenter::onFilterApplied);

connect(model, &ParsedLogData::fileLoaded,
        this, &MainPresenter::onFileLoaded);
```

### Strategy Pattern Implementation

**Strategy Interface**: `LogFilterCriteria` (defines matching logic)

**Context**: `ParsedLogData` (uses filter to produce filtered results)

**Concrete Strategies** (for future extension):
- `LevelFilterStrategy`: Matches by log level
- `TagFilterStrategy`: Matches by tag presence
- `RegexFilterStrategy`: Matches by regex pattern
- `CompositeFilterStrategy`: Combines multiple filters with AND/OR logic

---

## Validation Rules Summary

### Cross-Entity Validation

| Entity | Critical Validation | Error Handling |
|---------|---------------------|-----------------|
| LogEntry | Non-negative line numbers, non-empty required fields | Skip malformed line (spec FR-006) |
| LogFilterCriteria | Valid PCRE regex, valid color values | Show validation dialog (spec FR-020) |
| ColorScheme | Valid QColor values, non-transparent | Reset to defaults if invalid |
| WindowSettings | Valid geometry, positive column widths | Use defaults if corrupted |
| ParsedLogData | Consistency between counts and actual data | Recalculate on mismatch |

### Thread Safety

- `ParsedLogData` uses `std::mutex` for thread-safe access
- Filter operations are atomic (clear old, apply new)
- UI updates happen on main thread via Qt signals

---

## Performance Considerations

### Memory Efficiency

- **Small String Optimization (SSO)**: `std::string` for level, tag, content fields
- **Zero-Copy Views**: `std::string_view` for parsing and filtering operations
- **Lazy Loading**: `ParsedLogData` only loads visible entries (incremental fetch)

### Cache-Friendly Layout

- **Contiguous Memory**: `std::vector<LogEntry>` provides cache-friendly iteration
- **Data Locality**: Related fields (level, tag, content) stored together in struct
- **Index Mapping**: `filtered_indices` for O(1) filtered access

### Alignment with Constitution

- **Memory Constraint**: ≤ 1.5x file size (achieved through SSO and zero-copy)
- **Performance**: 3s load 5MB, 500ms filter response (achieved through chunked loading and lazy evaluation)

---

## Next Steps

1. Implement entities in `src/models/` directory
2. Create unit tests for each entity (≥ 90% coverage required)
3. Integrate entities with Qt signals/slots for Observer pattern
4. Implement serialization for persistence (QSettings compatibility)
5. Add performance benchmarks to validate constitution requirements

---

**Data Model Version**: 1.0
**Status**: Ready for implementation
**Next Phase**: Generate contracts (MVP interfaces)
