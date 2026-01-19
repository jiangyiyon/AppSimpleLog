<!--
SYNC IMPACT REPORT
==================
Version Change: 1.0.0 → 2.0.0 (MAJOR UPDATE)

Modified Principles:
- I. Feature-Driven Development → I. 版本控制与模式演进 (completely replaced)
- II. Specification First → II. 设计模式驱动开发 (completely replaced)
- III. Implementation Independence → III. Model-View-Presenter架构原则 (completely replaced)
- IV. Design Validation → IV. 设计模式文档规范 (completely replaced)
- V. Incremental Delivery → V. 增量交付策略 (completely replaced)

Added Sections:
- VI. 技术实施规范 (性能、UI、日志、搜索)
- VII. 产品增量计划 (4个增量及模式应用)
- VIII. 代码质量规范 (Google C++风格、模式文档、重构纪律)
- IX. 质量保证规范 (测试策略、覆盖率、文档)
- X. 验收标准规范 (增量验收、宪法修订流程)

Removed Sections: N/A

Templates Updated:
- .specify/templates/plan-template.md ⚠ NEEDS UPDATE - Must add design pattern gates
- .specify/templates/spec-template.md ⚠ NEEDS UPDATE - Must add pattern requirements
- .specify/templates/tasks-template.md ⚠ NEEDS UPDATE - Must add pattern validation tasks
- .specify/templates/agent-file-template.md ⚠ NEEDS UPDATE - Must add pattern guidance

Follow-up TODOs:
- Update plan-template.md to include Design Pattern Constitution Check section
- Update spec-template.md to include Design Pattern requirements
- Update tasks-template.md to include Pattern Validation and Documentation tasks
- Create pattern-specific documentation templates in .specify/templates/

Rationale for Version 2.0.0:
- MAJOR: Completely replaced all core principles with design pattern-focused governance
- MAJOR: Added comprehensive technical implementation standards (performance, UI, logging, search)
- MAJOR: Established incremental delivery plan with specific pattern applications
- MAJOR: Added detailed quality assurance and acceptance criteria
- This represents a fundamental shift from general workflow to pattern-driven architecture
-->

# SimpleLogView Constitution

## Core Principles

### I. 版本控制与模式演进

所有开发必须遵循严格的版本控制和模式演进纪律:
- 每个功能在独立的feature分支开发
- 模式重构必须在独立提交中，与功能开发分离
- main分支仅包含经过模式验证的代码
- 提交信息格式：[模式名称] 变更描述（如：[MVP] 重构搜索Presenter）

**Rationale**: 版本控制确保模式演进的可追溯性，分支隔离保证功能与重构的独立性，提交格式清晰标识模式变更。

### II. 设计模式驱动开发 (NON-NEGOTIABLE)

所有功能必须通过设计模式驱动的方式实现:
- SpecKit工作流必须包含设计模式分析阶段
- 每个用户故事必须明确说明应用的设计模式
- 模式参与者角色必须在代码文档中明确标注
- 代码审查必须包含设计模式应用检查清单

**代码审查清单（设计模式重点）:**
- [ ] 正确应用了设计模式
- [ ] 模式实现符合规范
- [ ] 通过所有模式验证测试
- [ ] 性能基准测试达标（3秒加载5MB）
- [ ] 模式文档完整

**Rationale**: 设计模式驱动确保代码架构的一致性、可维护性和可扩展性，防止临时设计和反模式的引入。

### III. Model-View-Presenter架构原则

所有UI组件必须严格遵循MVP模式职责分离:
- **Model**: 数据和业务逻辑（日志文件加载、解析、搜索算法）
- **View**: 界面显示和用户交互（Qt Widgets、布局管理）
- **Presenter**: 协调逻辑（连接Model和View，处理用户事件）
- 模式原则：每个界面组件必须对应明确的View职责
- 禁止跨层直接访问（View不能直接操作Model）

**Rationale**: MVP模式实现关注点分离，提高代码可测试性，支持界面和业务逻辑的独立演进。

### IV. 设计模式文档规范

所有设计模式必须有完整的文档:
- 每个类必须在头文件注释中说明应用的设计模式
- 模式参与者的角色必须明确标注（如：ConcreteStrategy、Observer等）
- 模式结构图建议使用PlantUML嵌入注释
- 模式变更必须更新相应文档
- 架构决策记录(ADR)必须包含模式选择理由

**设计文档必须包含:**
- 意图（Intent）
- 动机（Motivation）
- 结构图（Structure Diagram）
- 参与者（Participants）
- 协作方式（Collaborations）

**Rationale**: 完整的模式文档确保团队对架构理解一致，支持知识传递和后续维护。

### V. 增量交付策略

功能必须按增量计划逐步交付，每个增量聚焦特定模式应用:
- 每个增量必须实现完整的MVP模式结构
- 增量交付必须满足所有验收标准
- 增量演示必须展示模式应用的优势
- 增量之间保持向后兼容
- 每个增量完成后进行模式验证测试

**Rationale**: 增量交付降低风险，早期验证模式应用的正确性，确保持续交付价值。

## 技术实施规范

### VI. 性能约束规范

所有功能必须满足以下性能指标:
- **文件加载**: 加载5MB纯文本日志文件 ≤ 3秒
- **内存占用**: 加载后 ≤ 文件大小 × 1.5
- **界面响应**: 用户操作延迟 ≤ 100ms
- **搜索响应**: 实时更新 ≤ 500ms

**性能测试要求:**
- 每个增量必须通过性能基准测试
- 性能退化必须立即修复
- 性能优化必须在独立提交中

### VII. 界面设计规范

所有UI实现必须遵循以下规范:
- **框架**: Qt 6.10 Widgets模块
- **布局**: 支持自定义布局（可拖拽面板、可调整分割器）
- **主题**: 深色/浅色主题切换
- **配置**: 字体和颜色可配置
- **模式**: 每个界面组件必须对应明确的View职责

**UI组件模式应用:**
- 组合模式（Composite）：界面组件树结构
- 备忘录模式（Memento）：布局状态保存和恢复
- 观察者模式（Observer）：主题切换通知

### VIII. 日志处理规范

日志文件处理必须遵循以下规范:
- **格式**: 仅支持纯文本格式（UTF-8编码）
- **加载**: 必须支持大文件分块加载
- **监控**: 实时文件监控（tail -f功能）
- **导航**: 行号显示和快速导航

**日志处理模式应用:**
- 观察者模式（Observer）：文件变化通知
- 策略模式（Strategy）：不同的解析策略
- 建造者模式（Builder）：日志条目构建

### IX. 搜索过滤规范

搜索和过滤功能必须满足以下规范:
- **搜索**: 正则表达式搜索（支持PCRE语法）
- **高亮**: 关键字高亮（多颜色支持）
- **过滤**: 多条件组合过滤（AND/OR逻辑）
- **保存**: 保存和加载搜索条件
- **统计**: 搜索结果统计显示

**搜索过滤模式应用:**
- 策略模式（Strategy）：可替换的搜索算法
- 命令模式（Command）：搜索操作封装
- 组合模式（Composite）：多条件过滤器组合
- 装饰器模式（Decorator）：搜索结果高亮

## 产品增量计划

### 增量1.0：基础查看器 - MVP模式基础

**目标**: 实现基本的日志文件查看功能

**设计模式应用:**
- **Model**: 日志文件加载和基础解析（观察者模式 Observer）
- **View**: 文本显示区域和基本控件（组合模式 Composite）
- **Presenter**: 文件打开和显示协调（中介者模式 Mediator）

**验收标准:**
- 能打开5MB文件并在3秒内显示
- 符合MVP模式规范
- 支持基础文本显示
- 通过模式验证测试

### 增量2.0：搜索功能 - 策略模式应用

**目标**: 实现正则表达式搜索和高亮功能

**设计模式应用:**
- **Model扩展**: 正则表达式搜索算法（策略模式 Strategy）
- **View扩展**: 搜索栏和结果高亮
- **Presenter扩展**: 搜索协调逻辑（命令模式 Command）

**验收标准:**
- 支持实时搜索和高亮
- 搜索算法可替换（策略模式验证）
- 搜索响应 ≤ 500ms
- 通过模式验证测试

### 增量3.0：过滤系统 - 组合模式应用

**目标**: 实现多条件组合过滤功能

**设计模式应用:**
- **Model扩展**: 多条件过滤逻辑（组合模式 Composite）
- **View扩展**: 过滤条件输入界面
- **Presenter扩展**: 过滤协调

**验收标准:**
- 支持AND/OR逻辑过滤
- 过滤器可组合（组合模式验证）
- 过滤响应 ≤ 500ms
- 通过模式验证测试

### 增量4.0：布局自定义 - 备忘录模式应用

**目标**: 实现用户自定义界面布局功能

**设计模式应用:**
- **View扩展**: 可拖拽面板和布局保存（备忘录模式 Memento）
- **Presenter扩展**: 布局状态管理
- **Model扩展**: 布局配置持久化

**验收标准:**
- 用户可自定义界面布局
- 支持布局保存和恢复
- 布局状态可序列化（备忘录模式验证）
- 通过模式验证测试

## 代码质量规范

### X. Google C++风格指南特别条款

所有C++代码必须遵循Google C++风格指南:
- **C++20特性**: 使用C++20特性需团队批准
- **智能指针**: 优先unique_ptr，共享所有权需论证
- **异常处理**: 禁止异常，使用错误码或std::expected
- **头文件组织**: 必须遵循Google规范

**Rationale**: Google C++风格指南确保代码质量、一致性和可维护性。

### XI. 重构纪律（模式演进）

模式重构必须遵循严格的纪律:
- 模式重构必须小步进行，每次只改变一个模式维度
- 重构必须伴随测试，确保模式行为不变
- 重构前后必须有模式结构对比图
- 禁止在重构中添加新功能或改变模式职责
- 重构提交必须使用格式：[模式名] 重构说明

**重构流程:**
1. 编写或更新模式验证测试
2. 小步重构，每次提交一个模式变更
3. 运行所有测试确保行为不变
4. 更新模式文档和结构图
5. 代码审查通过后合并

### XII. Doxygen注释规范

所有类必须有完整的Doxygen注释:
- 必须标注应用的设计模式
- 必须标注模式中的角色（如：ConcreteStrategy、Observer等）
- 复杂算法必须有详细注释
- 公共接口必须有使用示例

**注释模板:**
```cpp
/**
 * @brief 类的简要描述
 *
 * @details 详细描述，包括设计模式应用
 *
 * @pattern 应用的设计模式名称
 * @role 在模式中的角色（如：ConcreteStrategy）
 *
 * @author 作者
 * @date 日期
 */
```

## 质量保证规范

### XIII. 模式验证测试策略

每个设计模式必须有完整的测试:
- **单元测试**: 每个模式必须有单元测试验证其正确性
- **集成测试**: 验证模式间协作
- **性能测试**: 每个模式实现必须满足性能指标
- **重构测试**: 确保重构不改变外部行为

**测试覆盖率要求:**
- **Model层**: ≥ 90%（业务逻辑核心）
- **Presenter层**: ≥ 85%（协调逻辑）
- **View层**: ≥ 70%（UI行为）
- **模式接口**: 100%覆盖

### XIV. 文档要求

所有设计和实现必须有完整的文档:
- **设计文档**: 每个设计模式必须有设计文档
  - 意图（Intent）
  - 动机（Motivation）
  - 结构图（Structure Diagram）
  - 参与者（Participants）
  - 协作方式（Collaborations）
- **代码文档**: 每个类必须有Doxygen注释，标注模式角色
- **架构决策**: ADR必须包含模式选择理由
- **用户文档**: 用户手册包含模式应用带来的好处说明

## 验收标准规范

### XV. 增量验收标准

每个增量必须满足以下条件:
1. **功能完整**: 从UI到数据层完整实现
2. **模式正确**: 设计模式应用正确，通过模式验证测试
3. **性能达标**: 性能指标达标（3秒加载5MB）
4. **代码质量**: 代码审查无模式应用问题
5. **演示成功**: 增量演示成功，展示模式优势

### XVI. 宪法修订流程

宪法本身的修订必须遵循以下流程:
- 任何规范变更必须通过团队评审
- 修订需记录原因和影响
- 宪法本身也需版本控制，与增量版本对齐
- 重大变更必须更新所有相关模板和文档

## Governance

### Amendment Procedure

- 宪法修订必须记录在案，使用语义版本控制
- **MAJOR**: 模式原则的重大变更、新增技术约束
- **MINOR**: 新增模式应用、扩展验收标准
- **PATCH**: 文档修正、措辞改进
- 修订必须更新所有相关模板
- 修订必须更新Last Amended日期（YYYY-MM-DD格式）

### Compliance Review

- 所有feature plan必须通过宪法检查
- 代码审查必须验证模式应用的正确性
- 性能测试必须通过才能合并
- 模式文档必须完整才能合并
- 不符合宪法的代码必须拒绝合并

### Runtime Guidance

- 开发团队必须参考feature-specific plan.md获取技术上下文
- 架构和设计决策必须记录在research.md
- 数据模型必须记录在data-model.md
- 设计模式应用必须记录在对应的pattern文档中
- 快速开始指南必须提供在quickstart.md

**Version**: 2.0.0 | **Ratified**: 2026-01-19 | **Last Amended**: 2026-01-19
