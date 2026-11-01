# Collector

Collector 是一个以 C++17 编写的 Linux 系统事件采集工程，包含用户态采集程序、eBPF 探针及加载/事件读取程序，并提供 GoogleTest 测试程序。项目按基础库分层、采集程序按职责拆分，便于复用底层能力和独立演进事件源、事件处理与控制逻辑。

> 当前构建配置和 eBPF 实现面向 Linux。仓库中存在少量其他平台相关代码，不代表当前完整工程已支持 macOS 或其他操作系统。

## 目录结构

### `src` 一级目录

| 目录 | 划分依据与职责 |
|---|---|
| [`src/base`](src/base) | 可供多个程序复用的基础库，按抽象层级组织；从通用算法和系统 API，逐层组合到 Collector 领域服务。 |
| [`src/proc`](src/proc) | 可执行程序及其专属源代码。每个一级子目录对应一个应用或工具，不与基础库的分层混在一起。 |
| [`src/third`](src/third) | 外部依赖源码或 Git 子模块，与项目自有实现分开管理。当前配置 GoogleTest 和 libbpf。 |

### `src/base` 分层及二级目录

各层按“通用程度与领域耦合度”递增划分：L1 不依赖 Collector 业务，L2 封装操作系统能力，L3 提供可复用服务，L4 实现 Collector 的业务与事件采集集成。目录表示代码组织意图；当前 CMake 会把部分层级源码直接编入各个可执行目标，并未通过独立库边界强制依赖方向。

#### L1：通用基础组件

| 目录 | 功能 |
|---|---|
| `src/base/L1/algorithm` | 通用算法和数据结构，如 Trie、前缀树及其他容器。 |
| `src/base/L1/base64` | Base64 编码/解码。 |
| `src/base/L1/jsoncpp` | 项目内的 JsonCpp 源码，用于 JSON 解析和生成。 |
| `src/base/L1/rapidxml` | RapidXML 及项目扩展，用于 XML 解析、构造和输出。 |
| `src/base/L1/string` | 字符串处理、转换、拆分和路径相关的字符串辅助功能。 |

#### L2：操作系统和运行时适配

| 目录 | 功能 |
|---|---|
| `src/base/L2/dns` | DNS 报文及名称解析辅助功能。 |
| `src/base/L2/file` | 文件系统操作、路径处理和文件信息查询。 |
| `src/base/L2/io-multiplexing` | I/O 多路复用抽象和事件等待处理。 |
| `src/base/L2/process` | 进程信息及进程相关操作。 |
| `src/base/L2/sleep` | 定时等待、休眠等辅助功能。 |
| `src/base/L2/system` | 操作系统、主机和运行环境信息。 |
| `src/base/L2/thread` | 线程封装和线程辅助设施。 |
| `src/base/L2/user` | 用户、组及 UID/GID 相关查询。 |

#### L3：通用服务与匹配能力

| 目录 | 功能 |
|---|---|
| `src/base/L3/config` | 配置文件读取和配置值管理。 |
| `src/base/L3/logger` | 日志初始化、输出和日志级别等管理。 |
| `src/base/L3/matcher` | 字符串、逻辑及 Trie 匹配器，供过滤条件复用。 |

#### L4：Collector 领域组件

| 目录 | 功能 |
|---|---|
| `src/base/L4/cache` | 进程等采集对象的缓存结构与管理。 |
| `src/base/L4/config` | Collector 运行配置及其访问接口。 |
| `src/base/L4/engines` | 事件采集引擎接口及平台/机制实现；二级目录如下。 |
| `src/base/L4/filter` | 按事件、字段和规则执行过滤。 |
| `src/base/L4/message` | 组件间消息定义、分发及消息中心。 |
| `src/base/L4/rule` | 过滤规则数据结构、XML 加载和序列化。 |
| `src/base/L4/schema` | 事件 schema 的初始化、校验和辅助处理。 |

`src/base/L4/engines` 的二级目录按采集机制划分：

| 目录 | 功能 |
|---|---|
| `bpf` | eBPF 对象、程序和链接管理；内部再按 tracepoint、kprobe 等类型组织。 |
| `es` | Apple Endpoint Security 相关实现；不是当前 Linux 构建的采集后端。 |
| `fanotify` | 基于 Linux fanotify 的文件事件采集。 |
| `netlink` | 基于 Linux connector/netlink 的进程事件采集。 |
| `udds` | Unix Domain Datagram Socket 事件接收实现。 |

#### 公共头文件

| 目录 | 功能 |
|---|---|
| `src/base/header/event` | 事件类型及事件 ID 定义。 |
| `src/base/header/schema` | 采集事件 schema 和事件体定义。 |
| `src/base/header` | 公共错误码等跨模块定义。 |

### `src/proc` 中的程序和模块

| 目录 | 程序/模块 | 功能 |
|---|---|---|
| `src/proc/collector` | `Collector` | 用户态主采集程序，负责初始化和运行控制器、插件、规则过滤和事件处理。 |
| `src/proc/collector-bpf` | `BPF` | Linux eBPF 探针源代码，由自定义 CMake 规则编译为 BPF 对象文件，不是独立用户态可执行程序。 |
| `src/proc/collector-bpf-loader` | `BPF-Loader` | 独立的 eBPF 加载/事件读取工具；加载选定探针、挂接采集点并从 perf buffer 读取事件，主要用于探针验证和调试。 |
| `src/proc/api-tester` | `API-Tester` | GoogleTest 测试程序，覆盖基础组件、过滤规则、匹配器、schema、消息及部分引擎接口。 |

#### Collector 内部目录

| 目录 | 功能 |
|---|---|
| `src/proc/collector/main` | `Collector` 入口、生命周期和进程级控制。 |
| `src/proc/collector/plugin` | 插件基类、插件事件接口及通用辅助设施。 |
| `src/proc/collector/plugins-data/cache` | 进程缓存插件及缓存更新、暂停/恢复等事件处理。 |
| `src/proc/collector/plugins-data/file` | 文件事件插件，处理读/写关闭和重命名等事件。 |
| `src/proc/collector/plugins-data/net` | 网络事件插件，处理连接及 DNS 相关事件。 |
| `src/proc/collector/plugins-data/proc` | 进程事件插件，处理 exec 等进程事件。 |
| `src/proc/collector/plugins-func/controller` | 插件控制器，负责插件初始化、规则加载和运行编排。 |
| `src/proc/collector/plugins-func/driver` | 驱动插件及事件引擎管理，负责事件源启停和注册。 |
| `src/proc/collector/plugins-func/proxy` | 插件间消息代理/转发。 |

#### eBPF 探针及测试目录

| 目录 | 功能 |
|---|---|
| `src/proc/collector-bpf/helpers` | BPF 编译兼容辅助头文件。 |
| `src/proc/collector-bpf/tp` | 基于 tracepoint 的探针，例如进程、connect、recv 和 rename 事件。 |
| `src/proc/collector-bpf/kp` | 基于 kprobe 的探针，以及针对内核布局/版本差异的适配头文件。 |
| `src/proc/collector-bpf/rs` | raw syscall 等其他类型的探针。 |
| `src/proc/api-tester/main` | GoogleTest 测试程序入口。 |
| `src/proc/api-tester/cases` | 按算法、文件、过滤、规则、schema、网络等模块组织的测试用例。 |

## 过滤规则

过滤 XML 结构、EventID/Object/Field/Matcher 编号、规则顺序、默认动作及当前限制详见[过滤规则说明](FILTER_RULES.md)。过滤器按 `EventID` 选择规则组，按规则 ID 升序判断，并以首条命中规则的动作作为结果。

仓库中的 `config/rule.xml` 是规则 XML 示例，不是 Collector 完整运行配置。当前 Collector 入口还会读取 `collector.json`；该配置文件目前不在仓库 `config/` 中，因此仅凭规则示例不能直接启动 Collector。

## 平台与构建依赖

当前 CMake 预设仅为 Linux 主机提供 `debug` 和 `release` 配置。构建整个默认工程还会配置并编译 eBPF 目标，需准备：

- CMake 3.22 或更高版本、Ninja。
- Clang/Clang++，以及 LLVM BPF 工具 `opt`、`llvm-dis`、`llc`。
- 与正在使用的内核相匹配的内核构建目录，通常为 `/lib/modules/$(uname -r)/build`。
- libbpf 构建依赖和程序链接依赖：libelf 开发文件、zlib 开发文件及 `pkg-config`。
- Git，用于初始化 GoogleTest 和 libbpf 子模块。

发行版软件包名称不同；例如 Debian/Ubuntu 系统通常需要安装 `cmake ninja-build clang llvm libelf-dev zlib1g-dev pkg-config`，并安装与当前运行内核匹配的内核头文件/构建包。请确认系统提供的 LLVM 工具包含 `opt`、`llvm-dis` 和 `llc`，且版本满足本机内核头文件和 BPF 编译要求。

加载 eBPF 通常需要 root 权限或相应 capabilities，且受内核版本、配置、架构及安全策略影响。项目当前根据运行内核版本查找内核 build 目录，并生成与该内核环境相关的 BPF 对象。

## 获取源码和构建

首次克隆时初始化子模块：

```sh
git clone --recurse-submodules <repository-url>
cd Collector
```

如果已经克隆源码但子模块目录未初始化：

```sh
git submodule update --init --recursive
```

当前 CMake 链接配置在构建目录中查找 `src/third/libbpf/src/libbpf.a`，但不会自动构建 libbpf。可按以下顺序配置并构建 Debug 版本：

```sh
cmake --preset debug
mkdir -p output/Debug/src/third/libbpf/src
make -C src/third/libbpf/src \
  BUILD_STATIC_ONLY=y \
  OBJDIR="$PWD/output/Debug/src/third/libbpf/src"
cmake --build output/Debug --parallel
```

Release 版本使用对应的构建目录：

```sh
cmake --preset release
mkdir -p output/Release/src/third/libbpf/src
make -C src/third/libbpf/src \
  BUILD_STATIC_ONLY=y \
  OBJDIR="$PWD/output/Release/src/third/libbpf/src"
cmake --build output/Release --parallel
```

构建产物通常位于 `output/Debug/collector/bin/` 或 `output/Release/collector/bin/`：

- `Collector`：用户态采集程序。
- `BPF-Loader`：独立 eBPF 探针加载/事件读取工具。
- `API-Tester`：GoogleTest 单元测试程序。
- `BPF/`：按 `tp/`、`kp/`、`rs/` 等子目录保存的 eBPF 对象文件。

由于顶层 CMake 无条件加入 BPF 子项目，即使只想构建 `API-Tester`，配置阶段仍需要可用的 BPF 工具链和内核 build 目录；`API-Tester` 的链接也依赖 libbpf 静态库。若配置或构建失败，请先检查上述工具是否在 `PATH` 中、内核 build 目录是否存在、子模块是否初始化，以及 libbpf 输出路径是否正确。

## 测试

当前 CMake 未注册 CTest 测试，构建后直接运行测试程序：

```sh
./output/Debug/collector/bin/API-Tester
```

按 GoogleTest 测试套件筛选，例如运行规则加载器相关用例：

```sh
./output/Debug/collector/bin/API-Tester --gtest_filter='RuleLoaderTest.*'
```

## 运行和配置说明

`BPF-Loader` 是低层探针测试工具，不等同于完整 Collector 服务。其第一个参数选择探针类型：`0` 对应 raw syscall 配置、`1` 对应 tracepoint、`2` 对应 kprobe；可选第二个参数按 `prog_args.hpp` 中的功能名筛选探针。运行前需先构建 BPF 对象，并通常需要 root 权限。例如，尝试加载 tracepoint 类的 connect 探针：

```sh
sudo ./output/Debug/collector/bin/BPF-Loader 1 connect
```

探针是否能成功加载和挂接取决于当前内核是否提供相应 hook、BPF 能力和权限。

完整 `Collector` 目前不提供可直接照抄的运行命令：入口期望可执行文件相邻的 `../config/collector.json`，而仓库当前只有规则示例；其启动分支也依赖命令行参数。请先准备与当前代码相符的 `collector.json`、确认规则路径及日志配置，再在隔离的 Linux 测试环境验证启动和退出行为。

## 第三方组件与许可证

- [`src/third/googletest`](src/third/googletest)：`API-Tester` 使用的 GoogleTest 子模块。
- [`src/third/libbpf`](src/third/libbpf)：eBPF 加载程序使用的 libbpf 子模块。
- `src/base/L1/jsoncpp`、`src/base/L1/rapidxml`：当前随项目源码管理的 JSON/XML 组件。

项目许可证见 [`LICENSE`](LICENSE) 和 [`COPYING`](COPYING)。第三方组件保留各自的许可证和版权要求；发布前应按其实际检出版本核查并保留相应声明。
