# Presenter Layer Interface

**Pattern**: Mediator (coordinates Model and View)
**Role**: ConcreteMediator

---

## IMainPresenter

```cpp
/**
 * @brief Interface for main presenter (MVP Mediator layer)
 *
 * @details Defines contract for coordinating between Model (data/logic)
 * and View (UI). Presenter contains business logic, handles user
 * events from View, and updates Model/View accordingly.
 *
 * @pattern Mediator
 * @role ConcreteMediator (Mediator pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IMainPresenter : public QObject {
    Q_OBJECT

public:
    virtual ~IMainPresenter() = default;

    // Initialization (from Application)
    /**
     * @brief Initialize presenter with Model and View instances
     * @param model Log data model
     * @param view Main view interface
     * @param settings Settings persistence
     */
    virtual void initialize(
        ILogModel* model,
        IMainView* view,
        ISettingsPersistence* settings
    ) = 0;

    // File operations (from View)
    /**
     * @brief Handle file open request from View
     * @param filePath Path to log file selected by user
     */
    virtual void openFile(const QString& filePath) = 0;

    /**
     * @brief Handle file open dialog request
     */
    virtual void openFileDialog() = 0;

    // Filtering operations (from View)
    /**
     * @brief Handle level filter change from View
     * @param levels Selected log levels
     */
    virtual void onLevelFilterChanged(const std::vector<std::string>& levels) = 0;

    /**
     * @brief Handle tag filter change from View
     * @param tags Selected tags
     */
    virtual void onTagFilterChanged(const std::vector<std::string>& tags) = 0;

    /**
     * @brief Handle regex filter request from View
     * @param pattern Regex pattern string
     */
    virtual void onRegexFilterRequested(const QString& pattern) = 0;

    // Sorting operations (from View)
    /**
     * @brief Handle table sort request from View
     * @param column Column index to sort
     * @param ascending Sort order
     */
    virtual void onSortRequested(int column, bool ascending) = 0;

    // Copy operation (from View)
    /**
     * @brief Handle copy request from View
     * @return Text to copy to clipboard
     */
    virtual QString getSelectedTextForCopy() = 0;

    /**
     * @brief Handle row selection from View
     * @param row Selected row index
     */
    virtual void onRowSelected(int row) = 0;

    // Settings operation (from View)
    /**
     * @brief Handle display settings dialog request from View
     */
    virtual void onDisplaySettingsRequested() = 0;

    /**
     * @brief Handle color scheme change from dialog
     * @param scheme New color scheme to apply
     */
    virtual void onColorSchemeChanged(const ColorScheme& scheme) = 0;

    // State queries (for testing/validation)
    /**
     * @brief Get current file path
     * @return Path to currently loaded file, or empty
     */
    virtual QString currentFilePath() const = 0;

    /**
     * @brief Get current filter criteria
     * @return Active filter configuration
     */
    virtual LogFilterCriteria currentFilter() const = 0;

    /**
     * @brief Get current color scheme
     * @return Active color scheme
     */
    virtual ColorScheme currentColorScheme() const = 0;

    /**
     * @brief Check if presenter is processing (file loading, filtering)
     * @return True if processing, false if idle
     */
    virtual bool isProcessing() const = 0;

    /**
     * @brief Check if any file is loaded
     * @return True if file is loaded, false otherwise
     */
    virtual bool hasFileLoaded() const = 0;
};
```

---

## IFilterPresenter

```cpp
/**
 * @brief Interface for filter settings presenter (MVP Mediator layer)
 *
 * @details Defines contract for managing filter dialog interactions,
 * coordinating between ColorScheme and FilterDialog.
 *
 * @pattern Mediator
 * @role ConcreteMediator (Mediator pattern participant)
 *
 * @author SimpleLogView Team
 * @date 2026-01-19
 */
class IFilterPresenter : public QObject {
    Q_OBJECT

public:
    virtual ~IFilterPresenter() = default;

    // Initialization
    /**
     * @brief Initialize presenter with dialog and settings
     * @param dialog Filter dialog interface
     * @param settings Settings persistence
     */
    virtual void initialize(
        IFilterDialog* dialog,
        ISettingsPersistence* settings
    ) = 0;

    /**
     * @brief Show filter dialog
     */
    virtual void showDialog() = 0;

    /**
     * @brief Handle color scheme change from dialog
     * @param scheme New color scheme
     */
    virtual void onColorSchemeChanged(const ColorScheme& scheme) = 0;

    /**
     * @brief Handle dialog acceptance
     * @param scheme Color scheme to save
     */
    virtual void onDialogAccepted(const ColorScheme& scheme) = 0;

    /**
     * @brief Handle dialog rejection
     */
    virtual void onDialogRejected() = 0;
};
```

---

## Signal/Slot Connections

### MainPresenter Signal Connections

```cpp
// Connect to Model (Observer pattern)
connect(model_, &ILogModel::dataChanged,
        this, &IMainPresenter::onDataChanged);

connect(model_, &ILogModel::filterApplied,
        this, &IMainPresenter::onFilterApplied);

connect(model_, &ILogModel::fileLoaded,
        this, &IMainPresenter::onFileLoaded);

connect(model_, &ILogModel::loadingProgress,
        this, &IMainPresenter::onLoadingProgress);

// Connect to View (Mediator pattern)
connect(view_, &IMainView::openFileRequested,
        this, &IMainPresenter::onOpenFileRequested);

connect(view_, &IMainView::levelFilterChanged,
        this, &IMainPresenter::onLevelFilterChanged);

connect(view_, &IMainView::tagFilterChanged,
        this, &IMainPresenter::onTagFilterChanged);

connect(view_, &IMainView::regexFilterRequested,
        this, &IMainPresenter::onRegexFilterRequested);

connect(view_, &IMainView::displaySettingsRequested,
        this, &IMainPresenter::onDisplaySettingsRequested);

connect(view_, &IMainView::sortRequested,
        this, &IMainPresenter::onSortRequested);

connect(view_, &IMainView::copyRequested,
        this, &IMainPresenter::onCopyRequested);

connect(view_, &IMainView::rowSelected,
        this, &IMainPresenter::onRowSelected);
```

### Presenter to View Slots

```cpp
// Update View based on Model changes
view_->updateData(model_->filteredCount());
view_->showStatus(QString::number(model_->filteredCount()) + " entries");
view_->applyColorScheme(currentColorScheme());
view_->showError(lastError());
view_->showLoadingProgress(loadedLines, totalLines);
view_->hideLoadingProgress();
view_->setWindowTitle("SimpleLogView - " + QFileInfo(filePath).fileName());
```

### Presenter to Model Methods

```cpp
// Trigger Model operations
model_->loadFile(filePath);
model_->applyFilter(currentFilter);
model_->clearFilter();
QStringList levels = toQStringList(model_->detectedLevels());
QStringList tags = toQStringList(model_->detectedTags());
```
