# View Layer Interface

**Pattern**: Composite (UI components)
**Role**: ConcreteObserver (Observer pattern participant)

---

## IMainView

```cpp
/**
 * @brief Interface for main application view (MVP View layer)
 *
 * @details Defines contract for main window UI components including toolbar,
 * log table, status bar, and their interactions. View emits signals
 * for user actions and provides slots for Presenter updates.
 *
 * @pattern Observer
 * @role ConcreteObserver (Observer pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IMainView : public QWidget {
    Q_OBJECT

public:
    virtual ~IMainView() = default;

    // Toolbar actions (signals)
    /**
     * @brief Emitted when user requests to open log file
     */
    void openFileRequested();

    /**
     * @brief Emitted when level filter selection changes
     * @param levels Selected level names (empty = all levels)
     */
    void levelFilterChanged(const std::vector<std::string>& levels);

    /**
     * @brief Emitted when tag filter selection changes
     * @param tags Selected tag names (empty = all tags)
     */
    void tagFilterChanged(const std::vector<std::string>& tags);

    /**
     * @brief Emitted when user requests regex filter application
     * @param pattern Regex pattern string
     */
    void regexFilterRequested(const QString& pattern);

    /**
     * @brief Emitted when user requests display settings dialog
     */
    void displaySettingsRequested();

    // Table actions (signals)
    /**
     * @brief Emitted when user sorts table by column
     * @param column Column index to sort by
     * @param ascending Sort order (true = asc, false = desc)
     */
    void sortRequested(int column, bool ascending);

    /**
     * @brief Emitted when user copies selected rows to clipboard
     */
    void copyRequested();

    /**
     * @brief Emitted when user selects a row
     * @param row Row index
     */
    void rowSelected(int row);

    // Update slots (from Presenter)
    /**
     * @brief Update table with new log data
     * @param entryCount Number of entries to display
     */
    virtual void updateData(size_t entryCount) = 0;

    /**
     * @brief Apply color scheme to UI elements
     * @param scheme Color scheme to apply
     */
    virtual void applyColorScheme(const ColorScheme& scheme) = 0;

    /**
     * @brief Show error message to user
     * @param message Error message to display
     */
    virtual void showError(const QString& message) = 0;

    /**
     * @brief Show loading progress to user
     * @param current Number of lines loaded
     * @param total Total lines to load
     */
    virtual void showLoadingProgress(int current, int total) = 0;

    /**
     * @brief Hide loading progress indicator
     */
    virtual void hideLoadingProgress() = 0;

    /**
     * @brief Show status message (e.g., "Loaded 1000 entries")
     * @param message Status message to display
     */
    virtual void showStatus(const QString& message) = 0;

    // Configuration (from Presenter)
    /**
     * @brief Enable/disable toolbar controls
     * @param enabled True to enable, false to disable
     */
    virtual void setControlsEnabled(bool enabled) = 0;

    /**
     * @brief Set window title
     * @param title Window title string
     */
    virtual void setWindowTitle(const QString& title) = 0;
};
```

---

## ILogTableView

```cpp
/**
 * @brief Interface for log table view (Composite UI component)
 *
 * @details Defines contract for displaying log entries in a table format.
 * Supports sorting, multi-selection, and row highlighting.
 *
 * @pattern Composite
 * @role LeafComponent (Composite pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class ILogTableView : public QWidget {
    Q_OBJECT

public:
    virtual ~ILogTableView() = default;

    // Data update (from Presenter)
    /**
     * @brief Set model for table data
     * @param model Log model providing data
     */
    virtual void setModel(QAbstractItemModel* model) = 0;

    // Display updates (from Presenter)
    /**
     * @brief Apply color scheme to table
     * @param scheme Color scheme to apply
     */
    virtual void applyColorScheme(const ColorScheme& scheme) = 0;

    /**
     * @brief Update row with highlight matches
     * @param row Row index to update
     * @param highlights Highlight positions and colors
     */
    virtual void updateRowHighlight(int row, const std::vector<HighlightMatch>& highlights) = 0;

    /**
     * @brief Update all rows with highlight matches
     * @param highlights Vector of highlight data per row
     */
    virtual void updateAllHighlights(const std::vector<std::vector<HighlightMatch>>& highlights) = 0;

    // Selection (signals)
    /**
     * @brief Emitted when user copies selected rows
     */
    void copySelectionRequested();

    /**
     * @brief Emitted when user selects a row
     * @param row Row index
     */
    void rowSelected(int row);

    // Sorting (signals)
    /**
     * @brief Emitted when user requests sort by column
     * @param column Column index
     * @param ascending Sort direction
     */
    void sortRequested(int column, bool ascending);
};
```

---

## IToolbarView

```cpp
/**
 * @brief Interface for toolbar view (Composite UI component)
 *
 * @details Defines contract for toolbar controls including file open,
 * level filter, tag filter, regex input, and settings button.
 *
 * @pattern Composite
 * @role LeafComponent (Composite pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IToolbarView : public QWidget {
    Q_OBJECT

public:
    virtual ~IToolbarView() = default;

    // Level filter (from Presenter)
    /**
     * @brief Update available levels in filter dropdown
     * @param levels Vector of level names
     * @param selected Currently selected levels
     */
    virtual void updateLevels(const QStringList& levels, const QStringList& selected) = 0;

    // Tag filter (from Presenter)
    /**
     * @brief Update available tags in filter dropdown
     * @param tags Vector of tag names
     * @param selected Currently selected tags
     */
    virtual void updateTags(const QStringList& tags, const QStringList& selected) = 0;

    // Regex input (from Presenter)
    /**
     * @brief Set regex pattern in input field
     * @param pattern Regex pattern string
     */
    virtual void setRegexPattern(const QString& pattern) = 0;

    /**
     * @brief Clear regex pattern
     */
    virtual void clearRegexPattern() = 0;

    // State (from Presenter)
    /**
     * @brief Enable/disable toolbar controls
     * @param enabled True to enable, false to disable
     */
    virtual void setEnabled(bool enabled) = 0;

    // Signals (to Presenter)
    /**
     * @brief Emitted when user clicks open file button
     */
    void openFileClicked();

    /**
     * @brief Emitted when user changes level filter selection
     * @param levels Selected level names
     */
    void levelFilterChanged(const QStringList& levels);

    /**
     * @brief Emitted when user changes tag filter selection
     * @param tags Selected tag names
     */
    void tagFilterChanged(const QStringList& tags);

    /**
     * @brief Emitted when user applies regex filter
     * @param pattern Regex pattern
     */
    void regexFilterApplied(const QString& pattern);

    /**
     * @brief Emitted when user clicks settings button
     */
    void settingsClicked();
};
```

---

## IStatusView

```cpp
/**
 * @brief Interface for status bar view (Composite UI component)
 *
 * @details Defines contract for displaying status messages, loading progress,
 * and entry counts to user.
 *
 * @pattern Composite
 * @role LeafComponent (Composite pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IStatusView : public QWidget {
    Q_OBJECT

public:
    virtual ~IStatusView() = default;

    /**
     * @brief Update status message
     * @param message Status text to display
     */
    virtual void setStatus(const QString& message) = 0;

    /**
     * @brief Clear status message
     */
    virtual void clearStatus() = 0;

    /**
     * @brief Show loading progress bar
     * @param current Current value
     * @param total Maximum value
     */
    virtual void showProgress(int current, int total) = 0;

    /**
     * @brief Hide loading progress bar
     */
    virtual void hideProgress() = 0;

    /**
     * @brief Update entry count display
     * @param total Total entries
     * @param filtered Entries matching current filter
     */
    virtual void updateEntryCount(int total, int filtered) = 0;
};
```

---

## IFilterDialog

```cpp
/**
 * @brief Interface for filter settings dialog (Composite UI component)
 *
 * @details Defines contract for color scheme and display settings dialog.
 * Allows users to customize colors for levels and regex highlights.
 *
 * @pattern Composite
 * @role LeafComponent (Composite pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IFilterDialog : public QDialog {
    Q_OBJECT

public:
    virtual ~IFilterDialog() = default;

    /**
     * @brief Set current color scheme for editing
     * @param scheme Color scheme to display
     */
    virtual void setColorScheme(const ColorScheme& scheme) = 0;

    /**
     * @brief Get modified color scheme
     * @return User-edited color scheme
     */
    virtual ColorScheme colorScheme() const = 0;

    /**
     * @brief Check if user confirmed changes
     * @return True if user clicked OK
     */
    virtual bool isAccepted() const = 0;

    // Signals
    /**
     * @brief Emitted when user confirms color scheme changes
     * @param scheme Modified color scheme
     */
    void colorSchemeChanged(const ColorScheme& scheme);
};
```
