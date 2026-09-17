# Lin 仓库的 Copilot 使用说明

目的：为后续 Copilot / Copilot CLI 会话提供简短、可执行的仓库特定指导，帮助高效开发。

---

1. 构建与运行（以开发为优先 — 默认不运行测试）

- 配置（常用，调试构建，不启用测试）：
  cmake --preset debug

- 构建：
  cmake --build --preset debug

- 运行可执行文件（构建后）：
  ./build/lin

- 如果确实需要构建包含测试的预设（仅在明确要求时使用）：
  cmake --preset debug-test
  cmake --build --preset debug-test

- 注意：
  - 本仓库的主要任务为开发工作。除非仓库所有者明确要求，否则不要在 Copilot 会话中运行、维护或优化测试。
  - CMakePresets.json 中包含测试相关预设；只有在被授权或明确需要时才使用 debug-test/release-test。
  - 未检测到统一的 lint 配置；仓库中没有默认的 lint 命令。

---

2. 高阶架构概览（关注多文件组合理解的要点）

- 顶层结构：
  - lib/: 核心代码，主要子域：Buffer（文本/网格模型）、Editor（业务逻辑、模式与命令）、UI（Window 与渲染粘合）。
  - assets/: 字体与 assets/resource.qrc（Qt 资源文件）。
  - main.cpp: 程序入口，创建 Lin::Editor::Context，调用 init() 和 run()，随后进入 QGuiApplication 事件循环。

- 运行时职责：
  - Editor/Context：负责应用逻辑与状态，初始化和顶层运行控制。
  - UI/Window：基于 Qt6::Gui 的渲染与输入承载层。
  - Buffer、Block：表示画布网格与逻辑块，由编辑器管理。

- 线程与通信：
  - 设计意图（见 doc/design.md）： 采用mvc架构, Context包含model和control, Window负责view. 通过Qt的事件驱动机制以及信号和槽来进行通讯交互

- 构建链：
  - C++17、Clang/clang++、LLD 链接器、Ninja、Qt6。CMake 已启用 AUTORCC 自动处理资源文件。

---

3. 仓库关键约定与模式

- 版本管理:
  - 不使用git进行版本管理, 动态开发

- 技术与约束（摘自 doc/design.md 与 doc/ai.md）：
  - 避免使用第三方库；优先使用 Qt 原生 API（在合适时使用 Qt 类型与惯用法优先于 std::）。
  - 编译器与链接器偏好由 CMakePresets.json 指定（Clang + LLD）。
  - 项目说明建议禁用异常与 RTTI，采用 Qt 风格的错误处理和类型转换（例如 qobject_cast / qvariant 等）。

- 所有权与习惯用法：
  - 使用智能指针管理所有权（仓库示例使用 std::unique_ptr 用于 Context）。
  - 保持逻辑层与 UI 层分离；lib/Editor 应负责业务逻辑，不直接做绘制工作。

- 资源与配置：
  - fonts 与其他资源位于 assets/，通过 assets/resource.qrc 打包。
  - .lin 文件格式在 doc/design.md 中描述（基于 JSON 的压缩格式）。默认配置位于 $HOME/.config/lin/config.lua。

- 测试：
  - 仓库包含 Qt Test 框架与示例测试，但当前指令明确要求在日常开发中忽略测试。只有在你明确要求时才处理测试相关内容。

- 文件/块（Block）语义（在修改相关代码时必须遵守的领域规则）：
  - 块在保存时应不重叠；插入/覆盖行为在 doc/design.md 中有详细描述。修改块相关逻辑时需保持这些不变式。

- 语言偏好：
  - 仓库所有者偏好中文。与仓库所有者或开发者交互时请始终使用中文，除非另有明确要求。

---

4. 重点阅读入口（便于 Copilot 会话快速定位相关实现）

- 首先阅读以下文件以理解行为和接口：
  - lib/Editor/Context.h(.cpp)
  - lib/Buffer/*（Block, Canvas）
  - lib/UI/Window.*
  - main.cpp
  - CMakePresets.json（构建/测试预设）
  - doc/design.md 与 doc/ai.md（架构与开发偏好说明）

---

5. 从仓库文档汇总的重要 AI / 助手指令

- doc/ai.md 包含仓库所有者偏好：使用中文回复、避免第三方库、遵循 Qt 开发模式，并要求 AI 阅读 doc/* 并改进代码。Copilot 会话应遵守这些仓库特定偏好（在适用时）。

---

如果需要我把整份说明改为纯中文（包括保留的命令示例在内，或加入 IDE 启动配置如 VSCode launch.json / CLion run config、Sanitizers 或 Valgrind 的示例），请说明具体需要，我会按要求补充。
