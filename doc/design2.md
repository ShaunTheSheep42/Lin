### Buffer层(数据结构与算法)

Block
Char
Canvas

### Edit层

Context
FileManager
RegisterManager
Interpreter
UndoManager

### Python层(插件和配置)

接入python插件以及python环境

### UI层

Context
Theme(Font, Color, Icon)

### Lin(代表整个App)

Edit::Context
UI::Context
Plugins
Config(通过python模块解析)

```
|------------------------------|
|              UI              |
|------------------------------|
|        |  A                  |
|    key |  | RenderContext    |
|        |  |                  |
|        v  |                  |
|------------------------------|
|            Edit              |
|------------------------------|
|                      |       |
|                      |       |
|------------------------------|
|        Context               |
|------------------------------|
|            Buffer            |
|------------------------------|
```
