# Model Layer Interface

**Pattern**: Observer (Subject)
**Role**: ConcreteSubject

---

## ILogModel

```cpp
/**
 * @brief Interface for log data model (Observer pattern Subject)
 *
 * @details Defines contract for log data management and change notification.
 * Implementations must emit signals when data changes and provide
 * thread-safe access to log entries.
 *
 * @pattern Observer
 * @role ConcreteSubject
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ILogModel : public QObject {
    Q_OBJECT

public:
    virtual ~ILogModel() = default;

    // Data access (read-only from View/Presenter)
    /**
     * @brief Get total number of entries
     * @return Entry count
     */
    virtual size_t entryCount() const = 0;

    /**
     * @brief Get entry at specific index
     * @param index Zero-based entry index
     * @return Log entry at index (throws if invalid)
     */
    virtual const LogEntry& entryAt(size_t index) const = 0;

    /**
     * @brief Check if more data can be lazy-loaded
     * @return True if additional data available
     */
    virtual bool canFetchMore() const = 0;

    /**
     * @brief Load next chunk of entries (lazy loading)
     * @param count Maximum entries to load
     */
    virtual void fetchMore(size_t count = 100) = 0;

    // Filtering (write-only from Presenter)
    /**
     * @brief Apply filter criteria to log entries
     * @param filter Filter conditions to apply
     * @return Number of matching entries
     */
    virtual size_t applyFilter(const LogFilterCriteria& filter) = 0;

    /**
     * @brief Clear current filter and show all entries
     */
    virtual void clearFilter() = 0;

    /**
     * @brief Get current filter statistics
     * @return Number of entries matching current filter
     */
    virtual size_t filteredCount() const = 0;

    // File operations (write-only from Presenter)
    /**
     * @brief Load log file from disk
     * @param filePath Path to log file (.log extension enforced)
     * @return True if load started successfully
     */
    virtual bool loadFile(const QString& filePath) = 0;

    // Metadata (read-only from Presenter/View)
    /**
     * @brief Get detected log levels in file
     * @return Vector of level names (case-insensitive)
     */
    virtual std::vector<std::string> detectedLevels() const = 0;

    /**
     * @brief Get detected tags in file
     * @return Vector of tag names
     */
    virtual std::vector<std::string> detectedTags() const = 0;

    /**
     * @brief Get parse statistics
     * @return Total, valid, malformed line counts
     */
    virtual ParseStatistics statistics() const = 0;

    // Signals (Observer pattern - notifies View/Presenter)
signals:
    /**
     * @brief Emitted when data changes (entries added/cleared)
     */
    void dataChanged();

    /**
     * @brief Emitted when filter is applied
     * @param matchCount Number of entries matching filter
     */
    void filterApplied(size_t matchCount);

    /**
     * @brief Emitted when file loading completes
     * @param success True if file loaded, false on error
     * @param error Error message if success is false, empty otherwise
     */
    void fileLoaded(bool success, const QString& error);

    /**
     * @brief Emitted during file loading (progress update)
     * @param linesLoaded Number of lines parsed so far
     * @param totalLines Total lines in file
     */
    void loadingProgress(int linesLoaded, int totalLines);
};
```

---

## ILogParser

```cpp
/**
 * @brief Interface for log line parsing (Strategy pattern)
 *
 * @details Defines contract for parsing log entries from raw text lines.
 * Different parsing strategies can be plugged in (e.g., for different
 * log formats).
 *
 * @pattern Strategy
 * @role ConcreteStrategy
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ILogParser {
public:
    virtual ~ILogParser() = default;

    /**
     * @brief Parse a single log line
     * @param line Raw log line string
     * @return Parsed log entry, or nullopt if line is invalid
     */
    virtual std::optional<LogEntry> parseLine(std::string_view line) const = 0;

    /**
     * @brief Validate log format before parsing
     * @param line Raw log line string
     * @return True if line appears valid for parsing
     */
    virtual bool isValidFormat(std::string_view line) const = 0;

    /**
     * @brief Get parser name for diagnostics
     * @return Human-readable parser identifier
     */
    virtual std::string parserName() const = 0;
};
```

---

## IFilterStrategy

```cpp
/**
 * @brief Interface for log entry filtering (Strategy pattern)
 *
 * @details Defines contract for filtering log entries based on criteria.
 * Concrete implementations can include level filtering, tag filtering,
 * regex matching, or composite combinations.
 *
 * @pattern Strategy
 * @role ConcreteStrategy
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IFilterStrategy {
public:
    virtual ~IFilterStrategy() = default;

    /**
     * @brief Check if log entry matches filter criteria
     * @param entry Log entry to evaluate
     * @return True if entry matches, false otherwise
     */
    virtual bool matches(const LogEntry& entry) const = 0;

    /**
     * @brief Get strategy description for UI display
     * @return Human-readable strategy description
     */
    virtual QString description() const = 0;

    /**
     * @brief Validate strategy configuration
     * @return True if strategy is validly configured
     */
    virtual bool isValid() const = 0;
};
```

---

## IColorSchemeProvider

```cpp
/**
 * @brief Interface for color scheme access (Memento pattern)
 *
 * @details Defines contract for retrieving UI colors for log levels
 * and highlights. Enables different color themes.
 *
 * @pattern Memento
 * @role Caretaker (Memento pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IColorSchemeProvider {
public:
    virtual ~IColorSchemeProvider() = default;

    /**
     * @brief Get color for specific log level
     * @param level Log level name (case-insensitive)
     * @return Color for displaying this level
     */
    virtual QColor levelColor(const std::string& level) const = 0;

    /**
     * @brief Get regex highlight color
     * @return Color for highlighting regex matches
     */
    virtual QColor regexHighlightColor() const = 0;

    /**
     * @brief Get background color
     * @return Background color for table/list
     */
    virtual QColor backgroundColor() const = 0;

    /**
     * @brief Get text color
     * @return Text color for log content
     */
    virtual QColor textColor() const = 0;

    /**
     * @brief Check if color scheme is valid
     * @return True if all colors are valid
     */
    virtual bool isValid() const = 0;
};
```

---

## ISettingsPersistence

```cpp
/**
 * @brief Interface for settings persistence (Memento pattern)
 *
 * @details Defines contract for saving and loading application settings.
 * Supports window state, color schemes, and user preferences.
 *
 * @pattern Memento
 * @role Caretaker (Memento pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ISettingsPersistence {
public:
    virtual ~ISettingsPersistence() = default;

    /**
     * @brief Save window geometry and state
     * @param widget Widget to save
     * @param name Unique identifier for this widget
     */
    virtual void saveWindowGeometry(QWidget* widget, const QString& name) = 0;

    /**
     * @brief Restore window geometry and state
     * @param widget Widget to restore
     * @param name Unique identifier for this widget
     */
    virtual void restoreWindowGeometry(QWidget* widget, const QString& name) = 0;

    /**
     * @brief Save color scheme
     * @param scheme Color scheme to persist
     */
    virtual void saveColorScheme(const ColorScheme& scheme) = 0;

    /**
     * @brief Load color scheme
     * @return Loaded color scheme (defaults if none saved)
     */
    virtual ColorScheme loadColorScheme() = 0;

    /**
     * @brief Save filter criteria
     * @param name Filter name (e.g., "SearchFilter", "LevelFilter")
     * @param filter Filter criteria to save
     */
    virtual void saveFilter(const QString& name, const LogFilterCriteria& filter) = 0;

    /**
     * @brief Load filter criteria
     * @param name Filter name to load
     * @return Loaded filter or empty if not found
     */
    virtual std::optional<LogFilterCriteria> loadFilter(const QString& name) const = 0;

    /**
     * @brief Save recent file path
     * @param filePath Path to log file
     */
    virtual void addRecentFile(const QString& filePath) = 0;

    /**
     * @brief Get list of recent files
     * @return List of recent file paths (most recent first)
     */
    virtual QStringList recentFiles() const = 0;
};
```
