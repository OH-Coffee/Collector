# Collector 过滤规则说明

本文说明 `rule` 模块如何读取 XML 规则，以及 `filter` 模块如何根据事件和采集字段执行匹配。规则中的数字 ID 和行为以当前实现为准。

## 工作流程

1. `rule::Loader` 从 XML 文件或字符串加载 `<FilterRules>`，并解析事件组、规则、条件及模式字符串。
2. `filter::Filter`（或底层 `filter::SubFilter`）读取 Loader 中的数据，为各条件创建字段提取器和匹配器。
3. 过滤时传入事件 ID 和该事件关联的进程、父进程及 schema 消息字段。过滤器只检查相同 `EventID` 的规则组。
4. 组内规则按 `Rule ID` 升序依次判断。首条匹配规则的 `Action` 即为结果，不再继续检查后续规则。

当前调用结果的含义：

- 命中 `Action="1"`：包含事件。
- 命中 `Action="2"`：排除事件。
- 没有匹配规则且未启用默认动作，或没有该事件的规则组：结果为未知。

## XML 结构

```xml
<FilterRules Version="1.0">
    <EventRuleGroup ID="1" EventID="1003">
        <Rule ID="1" Action="2" Relationship="1">
            <Condition ID="1" Object="101" Field="1004" Matcher="104">mv|bash</Condition>
        </Rule>
    </EventRuleGroup>
</FilterRules>
```

### 元素和属性

| 元素/属性 | 含义 |
|---|---|
| `FilterRules` | 规则文件根元素。 |
| `EventRuleGroup` | 针对一个事件 ID 的规则组。`ID` 是组 ID，`EventID` 是事件类型编号。一个 `EventID` 应只配置一个组；Loader 以事件 ID 为键保存组，重复事件 ID 会覆盖前一个组。 |
| `Rule` | 组内规则。`ID` 是规则 ID，`Action` 是命中动作，`Relationship` 是条件间关系。规则按 ID 升序评估，而不是按 XML 中的书写顺序。 |
| `Condition` | 单个字段匹配条件。`ID` 是条件 ID，`Object` 和 `Field` 共同标识待提取字段，`Matcher` 选择匹配方式，元素文本是匹配模式。 |

数值属性必须使用下表中的编号。实现也兼容旧字段名 `Feild`，但新规则应使用正确拼写 `Field`。

## EventID 编号

Linux 工程当前定义的事件 ID 如下：

| EventID | 事件 |
|---:|---|
| `1001` | FORK |
| `1002` | CLONE |
| `1003` | EXEC |
| `1004` | EXIT_GROUP |
| `1005` | EXEC_ENTRY |
| `2001` | OPEN |
| `2002` | RCLOSE |
| `2003` | WCLOSE |
| `2004` | RENAME |
| `2005` | RENAMEAT |
| `2006` | RENAMEAT2 |
| `2007` | VFS_RENAME |
| `3001` | CONNECT |
| `3002` | RECVFROM |
| `3003` | RECVMSG |
| `3004` | SKB_CONSUME_UDP |
| `9999` | GUDS（外部采集器事件） |

该编号表对应 Linux 下的 `EventType` 定义。macOS 使用 Endpoint Security 提供的事件编号，不能直接套用这张 Linux 编号表。

## 动作和逻辑关系

| 属性值 | 名称 | 行为 |
|---:|---|---|
| `Action="1"` | INCLUDE | 规则命中时包含事件。 |
| `Action="2"` | EXCLUDE | 规则命中时排除事件。 |
| `Relationship="1"` | AND | 规则中的所有条件都必须匹配。 |
| `Relationship="2"` | OR | 规则中的任一条件匹配即可。 |

规则按 ID 从小到大执行，所以更具体、优先级更高的规则通常应使用较小 ID。规则命中后不会再尝试后续规则。

## Object 和 Field 编号

条件通过 `Object` 与 `Field` 的组合选择字段。当前映射如下：

| Object | 对象 | 可用 Field |
|---:|---|---|
| `101` | 当前进程 | `1001` PID、`1002` 进程组 ID、`1003` 父进程 PID、`1004` 进程名称、`1005` 可执行文件路径、`1006` 命令行参数、`1007` 工作目录、`1008` UID、`1009` 会话 ID |
| `101` | 当前进程的可执行文件 | `2001` 文件名称、`2002` 文件路径 |
| `120` | 父进程 | `1001` PID、`1002` 进程组 ID、`1003` 父进程 PID、`1004` 进程名称、`1005` 可执行文件路径、`1006` 命令行参数、`1007` 工作目录、`1008` UID、`1009` 会话 ID |
| `121` | 父进程的可执行文件 | `2001` 文件名称、`2002` 文件路径 |
| `200` | 文件事件中的文件 | `2001` 文件名称、`2002` 文件路径 |
| `201` | 重命名后的目标文件 | `2001` 文件名称、`2002` 文件路径 |
| `300` | DNS 请求 | `4001` 请求域名、`4002` 请求结果、`4003` 应答内容、`4005` 应答类型 5 |
| `301` | 网络连接 | `3001` 协议、`3002` IPv6 标记、`3003` 入站标记、`3004` 源端口、`3005` 源 IP、`3006` 目标端口、`3007` 目标 IP |

注意：

- `Object="101"` 同时用于当前进程字段和当前进程可执行文件字段；使用不同的 `Field` 编号区分。
- DNS 应答字段在实现中有未完成/不支持的标记，不建议将 `4003` 或 `4005` 用于生产过滤规则。
- 未映射的 `Object`/`Field` 会提取为空字符串；这不等价于配置错误，匹配器仍会对空字符串执行判断。
- 某些事件产生时可能没有可用的父进程或网络字段，字段值取决于事件处理器实际提供的数据。

## Matcher 编号

普通匹配器支持以下编号。`|` 用于分隔多个模式；对于 `*_ANY` 类型，每个模式都会参与匹配。

| Matcher | 名称 | 行为 |
|---:|---|---|
| `101` | IS | 与单个模式完全相等。 |
| `102` | IS_ANY | 与任一模式完全相等。 |
| `103` | CONTAINS | 包含单个模式。 |
| `104` | CONTAINS_ANY | 包含任一模式。 |
| `105` | NOT | 不与单个模式完全相等。 |
| `106` | NOT_ANY | 不与任何一个模式完全相等。 |
| `107` | NOT_CONTAINS | 不包含单个模式。 |
| `108` | BEGIN_WITH | 以单个模式开头。 |
| `109` | BEGIN_WITH_ANY | 以任一模式开头。 |
| `110` | END_WITH | 以单个模式结尾。 |
| `111` | END_WITH_ANY | 以任一模式结尾。 |

另有基于 Trie 的多模式匹配：

| Matcher | 名称 | 行为 |
|---:|---|---|
| `201` | IS_ANY_CT | 字符串完全等于任一模式。 |
| `202` | IS_ANY_PT | 当前未实现；不要在规则中使用。 |
| `203` | BEGIN_WITH_ANY_CT | 字符串以任一模式为前缀。 |
| `204` | BEGIN_WITH_ANY_PT | 路径组件前缀匹配，分隔符为 `/`。 |
| `205` | END_WITH_ANY_CT | 字符串以任一模式为后缀。 |
| `206` | END_WITH_ANY_PT | 路径组件后缀匹配，分隔符为 `/`。 |

上述普通匹配和字符 Trie 匹配区分大小写。路径树匹配按 `/` 分段，不应将它们视为通用正则表达式。当前没有由规则 XML 暴露的正则表达式匹配模式。

## 示例

以下规则针对 Linux `EXEC` 事件（`EventID="1003"`）：先排除名称完全为 `blocked` 的进程，再包含名称中含 `agent` 或 `worker` 的进程。

```xml
<FilterRules Version="1.0">
    <EventRuleGroup ID="10" EventID="1003">
        <Rule ID="1" Action="2" Relationship="1">
            <Condition ID="1" Object="101" Field="1004" Matcher="101">blocked</Condition>
        </Rule>
        <Rule ID="2" Action="1" Relationship="1">
            <Condition ID="1" Object="101" Field="1004" Matcher="104">agent|worker</Condition>
        </Rule>
    </EventRuleGroup>
</FilterRules>
```

在此例中，`Object="101" Field="1004"` 选择当前进程名称；`Matcher="104"` 表示名称包含任一 `|` 分隔的模式。ID 为 1 的规则先执行：名称为 `blocked` 时排除；否则继续检查 ID 为 2 的规则。

如果一个规则有多个条件，`Relationship="1"` 要求全部匹配，`Relationship="2"` 则任一匹配即可。例如：

```xml
<Rule ID="3" Action="2" Relationship="1">
    <Condition ID="1" Object="101" Field="1004" Matcher="101">worker</Condition>
    <Condition ID="2" Object="101" Field="1005" Matcher="108">/tmp/</Condition>
</Rule>
```

该规则仅在进程名称完全为 `worker` 且进程路径以 `/tmp/` 开头时命中。

模式文本是 XML 文本节点。包含 `&`、`<` 等 XML 特殊字符时应使用 XML 实体（例如 `&amp;`、`&lt;`）。`|` 是模式列表分隔符，当前格式没有用于在单个模式中转义字面 `|` 的机制。

## 默认动作和未命中行为

默认情况下未启用默认动作。对已配置事件组的事件，如果没有规则命中，过滤器返回未知。

可通过 `rule::Loader::enable_default_on_match(true)` 启用默认动作。启用后，当该事件组没有规则命中时：

- 组内存在至少一条 INCLUDE 规则时，默认排除。
- 组内没有 INCLUDE 规则（例如全是 EXCLUDE 规则）时，默认包含。

默认动作仅用于已存在的事件组；没有对应事件组仍返回未知。调用 `filter::Filter::load(loader)` 时会将 Loader 的该设置传递给 Filter。

## 加载和更新

可通过 `rule::Loader::load_by_path(path, rule::MODE_RELOAD)` 加载文件，再调用 `filter::Filter::load(loader)` 建立过滤器。`rule::Loader::print()` 可将 Loader 当前已加载的数据序列化为 XML。

更新规则时，先重新加载 Loader，再使用 `filter::SubFilter::reload(loader)` 替换 SubFilter 中的旧规则。`SubFilter::load(loader)`（以及调用它的 `Filter::load(loader)`）会向现有过滤器追加解析器；重复加载时，SubFilter 应使用 `reload` 或先 `clear()`，Filter 则应创建新实例，避免重复执行旧规则。

虽然加载接口带有 `LoadMode` 参数，当前 Loader 实现每次解析根节点后都会清空原有规则；`MODE_APPEND` 目前不提供追加旧规则的效果。

## 当前实现注意事项

- 规则 XML 的解析会检查格式和根元素，但不会完整验证属性是否存在、编号是否有效或组合是否受支持。发布规则前应按本文的编号表检查。
- 每个 `EventID` 只能有一个有效规则组；重复的事件组会在 Loader 中相互覆盖。
- 规则和条件按 ID 升序排列，书写顺序不决定执行顺序。请确保同一组内 ID 唯一，避免相同 ID 的排序次序产生歧义。
- 每条规则至少应包含一个有效条件。当前 AND 逻辑对空条件集合会视为匹配，空规则可能导致无条件动作。
- `Matcher="202"` 未实现，执行时可能因缺少匹配器而失败，应避免使用。
- 该实现使用 XML 文本中的 `|` 拆分模式；重复条件的合并行为依赖相同的条件 ID、对象、字段和 matcher。建议用一个 `Condition` 携带多个 `|` 分隔模式，而不是依赖重复条件合并。

## 参考实现

- 字段 ID 和提取映射：`src/base/L4/filter/filter__mapping.hpp`、`src/base/L4/filter/filter__mapping.cpp`
- 动作、关系和 Matcher ID：`src/base/L4/filter/filter__executor.hpp`
- Matcher 创建及模式解释：`src/base/L4/filter/filter__executor.cpp`
- 规则 XML 加载和输出：`src/base/L4/rule/loader.cpp`
- 事件 ID：`src/base/header/event/event_type.h`
- 行为测试：`src/proc/api-tester/cases/filter/t_filter.cpp`、`src/proc/api-tester/cases/rule/t_rule_loader.cpp`
