# Quickstart Guide: Log Viewer with MVP Architecture

**Feature**: 1-log-viewer
**Generated**: 2026-01-19
**Purpose**: Setup development environment and build SimpleLogView application

---

## Prerequisites

### Software Requirements

- **Compiler**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2022 17.5+)
- **CMake**: Version 3.20 or higher
- **Qt**: Version 6.10 or higher
- **Operating System**:
  - Windows 10 or later (primary target)
  - Linux (GCC 10+)
  - macOS (Clang 12+)
- **Optional but Recommended**:
  - Git (for version control)
  - Visual Studio Code / CLion / Qt Creator (IDE)
  - Qt Creator (optional, any C++ IDE works)

### Hardware Requirements

- **Minimum**:
  - RAM: 4 GB
  - CPU: Dual-core (any modern processor)
  - Storage: 100 MB free disk space

- **Recommended**:
  - RAM: 8 GB or higher
  - CPU: Quad-core or higher
  - Storage: 500 MB free disk space

### Git Setup

```bash
# Clone repository (if not already cloned)
git clone <repository-url> SimpleLogView

# Navigate to project directory
cd SimpleLogView

# Create feature branch (already on 1-log-viewer)
git checkout -b 1-log-viewer

# Verify current branch
git branch
# Should show: * 1-log-viewer
```

---

## Project Setup

### 1. Directory Structure Verification

Verify that your working directory matches this structure:

```text
SimpleLogView/
├── .specify/                      # Specification and planning (already exists)
│   ├── memory/
│   │   └── constitution.md
│   └── templates/
│       └── ...
├── specs/                           # Feature specifications (already exists)
│   └── 1-log-viewer/
│       ├── spec.md
│       ├── plan.md
│       ├── research.md
│       ├── data-model.md
│       ├── quickstart.md          # This file
│       └── contracts/
│           ├── ILogModel.h
│           ├── IView.h
│           └── IPresenter.h
├── src/                            # Source code (to be created)
│   ├── models/
│   ├── views/
│   ├── presenters/
│   ├── utils/
│   ├── widgets/
│   └── main.cpp
├── tests/                          # Tests (to be created)
│   ├── unit/
│   ├── integration/
│   └── performance/
├── docs/                           # Documentation (to be created)
│   └── patterns/
├── resources/                       # UI resources (to be created)
│   ├── icons/
│   └── themes/
├── CMakeLists.txt                # Build configuration (to be created)
└── README.md                      # Project documentation (to be created)
```

### 2. Qt Environment Setup

#### Windows (MSYS2 or Qt Creator)

```bash
# Set Qt path in environment (replace with your Qt installation path)
export QTDIR="C:/Qt/6.10.0/msvc2019_64"
export PATH="$QTDIR/bin:$PATH"

# Verify Qt installation
qmake --version
# Should output: Qt version 6.10.x or higher
```

#### Linux (apt, dnf, pacman)

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Fedora/RHEL
sudo dnf install qt6-qtbase-devel qt6-tools-devel cmake gcc-c++

# Arch Linux
sudo pacman -S qt6-base qt6-tools cmake gcc
```

#### macOS (Homebrew)

```bash
# Install Qt 6 via Homebrew
brew install qt@6
brew install cmake

# Set Qt path
export CMAKE_PREFIX=/usr/local/Cellar/qt@6
```

---

## Building the Project

### 1. Generate Build System

From the project root, generate CMake build files:

```bash
# Create build directory
mkdir build
cd build

# Configure CMake (Debug build)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Configure CMake (Release build,recommended)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Or with specific Qt installation
cmake -DCMAKE_PREFIX=/path/to/qt -DCMAKE_BUILD_TYPE=Release ..
```

### 2. Compile the Application

```bash
# Build (use -j for parallel compilation)
cmake --build . --config Debug -- -j$(nproc)
# Or for Release
cmake --build . --config Release -- -j$(nproc)
```

### 3. Install/Run the Application

```bash
# Run from build directory
./src/SimpleLogView

# Or install to system
sudo cmake --install .
# Then run from anywhere
SimpleLogView
```

---

## Development Workflow

### Incremental Development Approach

Follow the incremental delivery plan defined in [plan.md](plan.md):

#### Phase 1: Basic Log Viewer (Weeks 1-2)

**Focus**: File loading and basic display
**Design Patterns**: Observer, Composite, Mediator

**Tasks**:
1. Implement `LogModel` (Model layer with Observer pattern)
2. Implement `LogTableView` (View layer with Composite pattern)
3. Implement `MainPresenter` (Mediator layer)
4. Implement log parser for `[LEVEL]TIME[PID,TID] tag:TAG MESSAGE` format
5. Implement chunked file loading with lazy population

**Acceptance Criteria**:
- Load 5MB file in ≤ 3 seconds
- Display entries in 6-column table
- Skip malformed lines and continue loading
- Support UTF-8 with Chinese/English characters

#### Phase 2: Search Functionality (Weeks 3-4)

**Focus**: Regex search and text highlighting
**Design Patterns**: Strategy, Command, Decorator

**Tasks**:
1. Implement `RegexSearchStrategy` (Strategy pattern)
2. Implement `SearchCommand` (Command pattern)
3. Implement regex input validation
4. Implement text highlighting with Decorator pattern
5. Add regex filter to existing filtering system

**Acceptance Criteria**:
- Support PCRE regex syntax
- Search within ≤ 500ms
- Highlight regex matches with custom color
- Handle invalid regex with error dialog

#### Phase 3: Filter System (Weeks 5-6)

**Focus**: Multi-condition level and tag filtering
**Design Patterns**: Composite, Strategy

**Tasks**:
1. Implement `CompositeFilter` (Composite pattern)
2. Implement `LevelFilterStrategy` and `TagFilterStrategy`
3. Implement multi-condition filter combination (AND/OR logic)
4. Add real-time filtering (immediate response < 500ms)

**Acceptance Criteria**:
- Filter by level (multiple selection)
- Filter by tag (multiple selection)
- Combine filters with AND/OR logic
- Real-time filter updates

#### Phase 4: Display Customization (Weeks 7-8)

**Focus**: Window state persistence and color schemes
**Design Patterns**: Memento, Observer

**Tasks**:
1. Implement `WindowMemento` (Memento pattern)
2. Implement `ColorScheme` with QSettings persistence
3. Implement theme switching (dark/light)
4. Implement high-DPI asset support (@2x naming)

**Acceptance Criteria**:
- Save/restore window geometry and state
- Persist column widths and sort orders
- Support custom color schemes
- High-DPI support across different displays

---

## Testing

### Run Unit Tests

```bash
# From build directory
cd build

# Run all unit tests
ctest --output-on-failure

# Run specific test suite
./tests/unit/test_LogModel

# Run with verbose output
ctest -V
```

### Run Performance Tests

```bash
# Run file loading performance benchmark
./tests/performance/test_FileLoading

# Run filter performance benchmark
./tests/performance/test_FilterPerformance

# Output should show:
# - 5MB file load time (must be ≤ 3000ms)
# - Filter response time (must be ≤ 500ms)
```

### Code Coverage

```bash
# Generate coverage report (Linux/macOS)
lcov --capture --directory ./src --output-file coverage.info
lcov --capture --directory ./tests --output-file coverage.info
lcov --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
xdg-open coverage_html/index.html  # Linux
open coverage_html/index.html          # macOS

# For Windows (requires compilation with --coverage flag)
# Use Visual Studio Code Coverage Tools or similar
```

---

## Common Development Tasks

### Adding New Features

1. **Update Specification**: Start with `/speckit.specify` if new user story needed
2. **Update Plan**: Run `/speckit.plan` to incorporate new requirements
3. **Update Contracts**: Add/update interfaces in `contracts/` directory
4. **Update Data Model**: Extend entities in `data-model.md`
5. **Generate Tasks**: Run `/speckit.tasks` to create task breakdown
6. **Implement Code**: Write code following Google C++ Style Guide
7. **Add Tests**: Write unit tests (Model: ≥ 90%, Presenter: ≥ 85%)
8. **Update Documentation**: Add Doxygen comments, update pattern docs
9. **Refactor**: Apply pattern refactoring discipline (small steps, tests first)

### Running the Application During Development

```bash
# Debug build with Qt Creator
# Open CMakeLists.txt in Qt Creator, click Build/Run

# Command line
./src/SimpleLogView

# With log file
./src/SimpleLogView --file path/to/example.log

# With custom settings (for testing)
./src/SimpleLogView --test-mode --log-file test.log
```

### Debugging

```bash
# Run with Qt debugger
gdb --args ./src/SimpleLogView --file test.log

# Run with Visual Studio Debugger
devenv ./src/SimpleLogView.exe --file test.log

# Enable Qt debug output
export QT_LOGGING_RULES="qt.core.*=true"
./src/SimpleLogView

# View Qt debug messages in console
export QT_LOGGING_TO_CONSOLE=true
./src/SimpleLogView
```

---

## Git Workflow

### Feature Branch Development

```bash
# Ensure on feature branch
git branch
# Should show: * 1-log-viewer

# View changes
git status

# Stage files for commit
git add src/ tests/ docs/ CMakeLists.txt

# Commit with pattern name (constitution requirement)
git commit -m "[MVP] Add basic log viewer with Observer pattern"
# Pattern name examples:
# [Strategy] Add regex search
# [Composite] Implement filter system
# [Memento] Add window persistence

# View commit history
git log --oneline

# Push to remote (if configured)
git push origin 1-log-viewer
```

### Pattern Refactoring Commits

```bash
# When refactoring to apply/fix a design pattern
git commit -m "[Observer] Refactor LogModel notification system for thread safety"

# When adding a new pattern
git commit -m "[Strategy] Add pluggable search algorithms"

# When documenting pattern changes
git commit -m "[Mediator] Update MainPresenter to coordinate multi-view architecture"
```

### Code Review Integration

```bash
# Before pushing, run code review checklist
# See constitution section: 代码审查清单（设计模式重点）

# Verify pattern application
# ✓ Correctly applied design patterns
# ✓ Pattern implementation follows conventions
# ✓ Pattern validation tests pass
# ✓ Performance benchmarks meet requirements (3s load 5MB)
# ✓ Pattern documentation is complete

# Then push
git push origin 1-log-viewer
```

---

## Troubleshooting

### Build Issues

**Problem**: CMake cannot find Qt

```bash
# Solution: Set Qt path explicitly
cmake -DCMAKE_PREFIX=/path/to/qt/6.10.0 ..

# Or set Qt_DIR
export Qt_DIR=/path/to/qt/6.10.0/lib/cmake/Qt6
cmake ..
```

**Problem**: C++20 features not recognized

```bash
# Solution: Verify compiler version and flags
g++ --version  # Should be GCC 10+, Clang 12+, or MSVC 2022 17.5+

# Add C++20 standard flag explicitly
cmake -DCMAKE_CXX_STANDARD=20 ..

# Check CMakeLists.txt for C++20 requirement
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Runtime Issues

**Problem**: Application crashes on file load

```bash
# Solution: Check for NULL pointers and exceptions
# Add debug output
qDebug() << "Loading file:" << filePath;

# Verify file exists before opening
QFile file(filePath);
if (!file.exists()) {
    qWarning() << "File does not exist:" << filePath;
    return;
}
```

**Problem**: Memory usage exceeds 1.5x file size

```bash
# Solution: Enable lazy loading and check for memory leaks
# Valgrind on Linux
valgrind --leak-check=full --show-leak-kinds=all ./src/SimpleLogView

# Visual Studio Diagnostics Tools on Windows
# Use CRT heap allocation tracking
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
```

**Problem**: UI is not responsive during filtering

```bash
# Solution: Move filtering to worker thread or use lazy evaluation
# Verify ranges are being used (C++20 feature)
# Profile filter operation
# Should complete within 500ms for large datasets
```

---

## Resources

### Documentation

- **C++20 Reference**: https://en.cppreference.com/w/cpp/20
- **Qt 6.10 Documentation**: https://doc.qt.io/qt-6/
- **Google C++ Style Guide**: https://google.github.io/styleguide/cppguide.html
- **Design Patterns**: "Head First Design Patterns" by Freeman & Freeman
- **Refactoring**: "Refactoring: Improving the Design of Existing Code" by Martin Fowler

### Project-Specific

- **Constitution**: [.specify/memory/constitution.md](../.specify/memory/constitution.md)
- **Feature Spec**: [spec.md](spec.md)
- **Implementation Plan**: [plan.md](plan.md)
- **Data Model**: [data-model.md](data-model.md)
- **Research**: [research.md](research.md)

### Community

- **Qt Forum**: https://forum.qt.io/
- **Stack Overflow**: https://stackoverflow.com/questions/tagged/qt6
- **C++ Slack**: #cpp-language channel on Slack

---

## Next Steps

1. **Create Source Structure**: Implement `src/models/`, `src/views/`, `src/presenters/` directories
2. **Implement Phase 1 Tasks**: Follow tasks from `/speckit.tasks` output
3. **Write Tests**: Create unit tests achieving ≥ 90% (Model) and ≥ 85% (Presenter) coverage
4. **Run Performance Benchmarks**: Validate 3s load time, 500ms filter response
5. **Code Review**: Verify pattern applications and design pattern compliance
6. **Document Patterns**: Create PlantUML diagrams in code comments, update pattern docs
7. **Commit and Push**: Use `[Pattern] Description` commit format per constitution

**Happy Coding!** 🚀
