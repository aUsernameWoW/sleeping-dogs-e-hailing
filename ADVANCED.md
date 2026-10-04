# SDTaxi — advanced users and developers

> [!IMPORTANT]
> **关于这个 mod**：它完全是用 Claude Code 里的 Claude Fable 和 Opus vibe coding 写出来的，几乎没有经过审查，请当作
> 实验性质的 mod 使用，发现异常请反馈。它在很大程度上依赖 [SDmodding](https://github.com/SDmodding) 的成果，SDmodding 几乎全部出自 [sneakyevil](https://github.com/sneakyevil)
> 之手，这个 mod 背后的逆向分析都从那里开始。完整致谢见页面底部的[致谢](#致谢)。
>
> **About this mod**: it was fully vibe-coded with Claude Fable and Opus in Claude Code, with little review, so treat
> it as experimental and please report anything unusual. It relies heavily on [SDmodding](https://github.com/SDmodding), almost entirely the work of
> [sneakyevil](https://github.com/sneakyevil): the reverse engineering behind it starts there. Full credits: [Credits](#credits), at the bottom.

[中文](#中文) | [English](#english)

新手安装说明见 [README.md](README.md)。 · Step-by-step install for players: [README.md](README.md).

## 中文

### 这个 mod 做了什么

游戏里本来就有一个会把车送到你面前的联系人：代客泊车（Car Valet）。这个 mod 照它的做法派一辆的士来，不同的是
司机留在车上，车停下来等你，你像拦路边的士一样上车，之后由游戏原本的坐的士流程接手。

- 通讯录里加一个联系人（默认名字 `Taxi`，头像用游戏的“未知联系人”头像）。不往游戏的进度里加东西，存档里不会
  留下它，卸载 mod 后也不会留下一个出错的联系人。
- 拨打：显示拨号界面，2 秒后挂断。在你周围找一个视野外、能刷车的路面位置（和代客泊车同样的搜索：40-70 m，
  找不到就逐步扩大到 150 m），刷出一辆的士（`625MHCTaxi01`）和的士司机（`TaxiDriverMale`），地图上标为友方。
- 的士开向离你最近的路面位置，每 5 秒按你当前的位置重新定一次路线，到你 12 m 以内就停下，一直停着等你上车。
  两分钟没到，或者停下后两分钟没人上车，它就像普通的士一样开走。
- 上车以后全是游戏原本的流程：叫车提示（按住 E）、车费、地图（的士视图）、确认后黑屏跳过路程、到站下车。
  地图标记在你上车时去掉。
- 一次只派一辆：车在路上或你在车上时联系人隐藏，结束后恢复。读档或场景重置会中断派车，联系人也会恢复。

### 原理

- 运行时编译执行 SkookumScript：游戏的 exe 里还带着 Skookum 编译器（`UFG::ScriptCache::GetScript` +
  `SkookumMgr::RunExternalCodeBlock`，游戏用它执行动作树里的脚本片段），mod 在脚本自己的 tick
  （`SkookumScript::update_delta`）里调用。派车是一段 Skookum 脚本（`core/taxi.cc`），照代客泊车的
  `CCAmbient._gameslice_main` 和交通脚本 `Tran._spawn_vehicle`（刷出带司机的的士）写成。
- 通讯录：hook `UIHK_PDAPhoneContactsWidget::PopulateList` 加联系人，`LaunchSubOption` 处理拨打（拨号界面照
  `LaunchCallMission` 的做法）。没有用脚本的 `PDA.add_contact`：它会把联系人记进游戏进度，可能随存档留下来。
- 日志：游戏脚本的 `Debug.print/println` 在发行版里是空函数，mod 把它们换成写日志的函数（游戏自己脚本的输出也会
  记下来）；脚本编译和运行错误经 `ADebug::print` 写进日志。坐车过程 hook `TransitUtility` 的几个函数和
  `AiDriverComponent::WarpToDestination`，只做记录。
- 函数都用在旧版 v1.0 和当前 Steam 版里都唯一的字节特征码定位，找不到时对应功能关闭，日志里写 `MISSING`。
  开头已被别的 mod 用 MinHook hook 过的函数（例如同时装了 [SDEncore](https://github.com/aUsernameWoW/sleeping-dogs-encore)，
  它先加载，hook 了五个相同的函数）也能找到，两个 mod 的 hook 串在一起；`Debug.println` 也是串联替换的。

调查过程和全部细节见 [CLAUDE.md](CLAUDE.md)（英文）。

### 日志

- `phone: taxi contact "Taxi" in the phone's contacts`：启动时，联系人已加好。
- `phone: the player called the taxi contact`、`taxi: dispatching ...`，然后 `script: [SDTaxi] ...`：每次叫车的经过
  （刷在哪、路上的车速和位置、到没到、你上没上车）；`taxi: dispatch over, the taxi contact is back`：结束。
- `ride:` 开头的行：坐车过程（选的目的地、`WarpToDestination` 跳过路程、车辆 AI 驾驶状态的变化）。
- `skookum:` 开头的行里有 `error`：脚本编译或运行出错（附带出错位置）。
- `scan: ... (it starts with a jump: hooked by another mod)`：这个函数别的 mod 先 hook 了，已串联。
- `crash:` 开头的行：崩溃时的位置和调用栈。游戏每次退出都会崩一次（原版问题），这一条可以忽略。

### 设置（`plugins\SDTaxi.ini`）

| 项 | 默认 | 说明 |
|---|---|---|
| `[Phone] Contact` | 1 | 通讯录里的叫车联系人。 |
| `[Phone] Name` / `Info` | Taxi / Call a cab to where you are | 联系人名字和说明。只能用游戏字体有的字符；`$KEY` 会查游戏自己的文本。 |
| `[Phone] Portrait` | Portrait_Smartphone_Unknown | 头像（手机联系人贴图包里的贴图名）。 |
| `[Taxi] Vehicle` | object-physical-vehicle-625MHCTaxi01 | 派来的车（属性集名）。绿色新界的士：`object-physical-vehicle-625MHCTaxiGreen01`。 |
| `[Taxi] Driver` | object-physical-character-TaxiDriverMale | 司机（属性集名）。 |
| `[Debug] Logging` | 1 | 写 `SDTaxi.log`；崩溃时另写 `SDTaxi-crash-<n>.dmp`。 |
| `[Debug] ScriptPrints` | 1 | 把游戏脚本的 `Debug.print/println`（发行版里是空函数）也写进日志。 |
| `[Debug] RideLog` | 1 | 记录坐车过程（目的地、跳过路程、车辆 AI 状态）。 |
| `[Debug] Console` / `ConsoleKey` | 0 / F11 | 开发用：按键执行 `SDTaxi-console.sk` 里的脚本，结果写进日志。 |

改完重启游戏生效。开发时，`plugins` 里放一个 `SDTaxi-dispatch.sk` 可以替换内置的派车脚本，每次拨打时重新读取
（占位符 `{HANGUP}`、`{VEHICLE}`、`{DRIVER}` 和内置脚本一样会被替换）。

### 需求与兼容性

- 《热血无赖：终极版》的两个发行版本（当前 Steam 版和旧版 v1.0，特征码在两者上都唯一匹配），Windows 10/11 x64。
  不依赖 Windows 专有服务，应当能在 Wine/Proton/CrossOver 下运行（未测试）。
- 任意 ASI 加载器，例如 [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)（`SDTaxi.zip`
  里自带一份，作为 `dinput8.dll`）。
- 不修改任何游戏文件。可以和 SDEncore 同时使用。

### 下载

[Releases](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/releases) 里每个版本都有 `SDTaxi.zip`（加载器 +
mod）、`SDTaxi.asi`（只有 mod）、`SDTaxi.pdb`（调试符号）和 `THIRD-PARTY-NOTICES.md`。`main` 上每次提交都会自动
编译、测试并发布为预发布版 `build-<N>`（没有在游戏里测过）；在游戏里验证过的构建会转为正式版，README 里的下载
链接指向最新的正式版。

### 编译与测试

Visual Studio 2022（v143），Windows SDK 10.0.26100。项目需要放在工作区的 `mods\SDTaxi`，工作区里还要有
`reference\minhook`（[MinHook](https://github.com/TsudaKageyu/minhook) v1.3.4 源码，随项目一起编译）。在工作区
根目录运行 `.\tools\build.ps1 -Mod SDTaxi -Test`：`load_test` 在游戏之外加载 .asi，不能崩溃，写出默认 ini，并报告
找不到游戏函数；`scan_test` 检查特征码查找，包括函数开头已被 hook 的情况。GitHub Actions 用同样的布局编译
（`-warnAsError`）、测试、打包并发布预发布版，依赖版本固定在 `.github/reference.env` 和 `.github/asi-loader.env`。

### 致谢

这个 mod 用到或参考了下面这些人和项目的成果，在此致谢。

**研究资料**

- [SDmodding](https://github.com/SDmodding)，几乎全部出自 [sneakyevil](https://github.com/sneakyevil) 一人之手。这个 mod 用到了：
  - SDmodding 随 [SDK](https://github.com/SDmodding/SDK) 发布的 [Visual Studio 2022 项目模板](https://github.com/SDmodding/SDK/releases/tag/vs2022)：这个 mod 的 Visual Studio 工程源自这个模板，编译设置和以 `dllmain.cc` 为起点的源文件结构都来自它；
  - SDmodding 分享的游戏 v1.0 版 exe 和调试符号（PDB，Steam 首发版自带）：游戏的脚本系统、手机通讯录和坐的士的流程都是从这里查到的；
  - [SDK](https://github.com/SDmodding/SDK)：游戏里的类名和数据结构；
  - [Files](https://github.com/SDmodding/Files) 里导出的属性集（的士司机就是在这里找到的）、动作树和符号表（QSymbolsDictionary）；
  - [BigFileSystem](https://github.com/SDmodding/BigFileSystem)、[TheoryEngine](https://github.com/SDmodding/TheoryEngine)，以及 sneakyevil 的 [SD-BigFileExplorer](https://github.com/sneakyevil/SD-BigFileExplorer) 和 [Ekey](https://github.com/Ekey) 的 SDDEUnpacker 里的文件名列表：
    读取游戏资源包（`.big`）的工具是照着它们写的，游戏的脚本和动作树都是用它从资源包里取出的。

**游戏原有的内容**

- 叫车的脚本照游戏里代客泊车联系人（Car Valet）和生成的士的脚本写成，它们由 United Front Games 编写，版权归
  Square Enix 所有。上车以后的叫车提示、车费、地图和跳过路程都是游戏原本的功能。
- SkookumScript（Agog Labs）：游戏的脚本语言，mod 用游戏自带的编译器运行叫车的脚本。

**mod 里包含的代码**（许可证全文见 [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)）

- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)（ThirteenAG）：压缩包里的 `dinput8.dll`，让游戏加载 mod。它本身还包含 MinHook、
  [miniz](https://github.com/richgel999/miniz)（Rich Geldreich 等）和 [praydog](https://github.com/praydog) 的 FunctionHookMinHook。
- [MinHook](https://github.com/TsudaKageyu/minhook)（Tsuda Kageyu，内含 Vyacheslav Patkov 的 Hacker Disassembler Engine）：mod 靠它接入游戏。

**工具**

- [IDA Pro](https://hex-rays.com/ida-pro)（Hex-Rays）和 [ida-pro-mcp](https://github.com/mrexodia/ida-pro-mcp)（mrexodia）：分析游戏程序。
- [Claude Code](https://claude.com/claude-code)（Anthropic）：这个 mod 完全是用 Claude Fable 和 Opus vibe coding 写出来的，代码、文档和逆向分析都出自 Claude，几乎没有经过人工审查。

**游戏与商标**

《热血无赖：终极版》（Sleeping Dogs: Definitive Edition）由 United Front Games 开发、Square Enix 发行，
游戏及其内容的版权归 Square Enix 所有。

与 Square Enix、United Front Games 均无关联。

## English

### What the mod does

The game already has a contact who brings a car to you: the Car Valet. This mod sends a taxi the same way, except
that the driver stays in, the cab stops and waits, you get in as you would with a cab on the street, and the game's
own taxi ride takes over from there.

- A contact in the phone (named `Taxi` by default, with the game's "unknown contact" portrait). Nothing is added to
  the game's progression: saves don't keep it, and removing the mod leaves no broken contact behind.
- Calling it: the call screen shows and hangs up after 2 s. The mod looks for an off-screen spot on a road around
  you where a car can spawn (the Car Valet's search: 40-70 m, widened step by step to 150 m), spawns a taxi
  (`625MHCTaxi01`) with a taxi driver (`TaxiDriverMale`) and marks it on the map as friendly.
- The taxi drives to the road position nearest to you, re-aimed every 5 s at where you are now, stops once it's
  within 12 m of you and stays there until you get in. If it doesn't reach you within two minutes, or nobody gets in
  for two minutes after it stopped, it drives off like any taxi.
- From getting in on it's all the game's: the hire prompt (hold E), the fare, the map (taxi view), the fade that
  skips the trip once you confirm, getting out at the destination. The map marker goes when you get in.
- One taxi at a time: while it's on its way or you're riding, the contact is hidden; it comes back afterwards.
  Loading a save or a scene reset ends a dispatch, and the contact comes back then too.

### How it works

- SkookumScript compiled and run at run time: the exe still contains the Skookum compiler
  (`UFG::ScriptCache::GetScript` + `SkookumMgr::RunExternalCodeBlock`, which the game uses for the script snippets in
  its action trees), called from the scripts' own tick (`SkookumScript::update_delta`). The dispatch is a Skookum
  script (`core/taxi.cc`) modeled on the Car Valet's `CCAmbient._gameslice_main` and the traffic script
  `Tran._spawn_vehicle` (which spawns a taxi with its driver).
- Contacts: a hook on `UIHK_PDAPhoneContactsWidget::PopulateList` adds the contact, `LaunchSubOption` handles the
  call (the call screen as `LaunchCallMission` shows it). Not the scripts' `PDA.add_contact`: that records the contact
  in the game's progression, where a save may keep it.
- Logging: the game scripts' `Debug.print/println`, empty functions in this build, are replaced by ones that write
  to the log (the game's own scripts' output included); script compile and run errors reach the log through
  `ADebug::print`. For the ride, a few `TransitUtility` functions and `AiDriverComponent::WarpToDestination` are
  hooked, only to log.
- Every function is found by a byte signature unique in both the legacy v1.0 and the current Steam build; a missing
  one turns its feature off and logs `MISSING`. Functions another mod already hooked with MinHook are still found (with
  [SDEncore](https://github.com/aUsernameWoW/sleeping-dogs-encore) installed too, it loads first and hooks five of the
  same functions), and both mods' hooks chain; so do the replaced `Debug.println`s.

The investigation and all details: [CLAUDE.md](CLAUDE.md).

### Log

- `phone: taxi contact "Taxi" in the phone's contacts`: at start, the contact is in.
- `phone: the player called the taxi contact`, `taxi: dispatching ...`, then `script: [SDTaxi] ...`: each call (where
  the taxi spawned, its speed and position on the way, whether it arrived, whether you got in);
  `taxi: dispatch over, the taxi contact is back`: the end.
- `ride:`: the ride (the destination picked, `WarpToDestination` skipping the trip, the vehicle AI's driving state as
  it changes).
- `skookum:` lines with `error`: a script failed to compile or run (with the position).
- `scan: ... (it starts with a jump: hooked by another mod)`: another mod hooked that function first; chained.
- `crash:`: where a crash happened, with the stack. The game crashes on every exit (an original bug): ignore that one.

### Settings (`plugins\SDTaxi.ini`)

| Setting | Default | |
|---|---|---|
| `[Phone] Contact` | 1 | The taxi contact in the phone. |
| `[Phone] Name` / `Info` | Taxi / Call a cab to where you are | The contact's name and info line. Only characters the game's fonts have; a `$KEY` looks up the game's own text. |
| `[Phone] Portrait` | Portrait_Smartphone_Unknown | The portrait (a texture of the phone contacts' pack). |
| `[Taxi] Vehicle` | object-physical-vehicle-625MHCTaxi01 | The vehicle sent (a property set). Green New Territories taxi: `object-physical-vehicle-625MHCTaxiGreen01`. |
| `[Taxi] Driver` | object-physical-character-TaxiDriverMale | The driver (a property set). |
| `[Debug] Logging` | 1 | Write `SDTaxi.log`; on a crash also `SDTaxi-crash-<n>.dmp`. |
| `[Debug] ScriptPrints` | 1 | Log the game scripts' `Debug.print/println` too (empty functions in this build). |
| `[Debug] RideLog` | 1 | Log taxi rides (destination, skipping the trip, the vehicle AI's state). |
| `[Debug] Console` / `ConsoleKey` | 0 / F11 | Development: a key runs the scripts in `SDTaxi-console.sk`; results go to the log. |

Restart the game after changing it. For development, an `SDTaxi-dispatch.sk` in `plugins` replaces the built-in
dispatch script and is read at every call (with the same placeholders filled in: `{HANGUP}`, `{VEHICLE}`, `{DRIVER}`).

### Requirements and compatibility

- Both releases of Sleeping Dogs: Definitive Edition (the current Steam build and the legacy v1.0; the signatures
  match uniquely in both), Windows 10/11 x64. No Windows-only services, so it should run under
  Wine/Proton/CrossOver (untested).
- Any ASI loader, e.g. [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) (`SDTaxi.zip`
  ships one as `dinput8.dll`).
- Changes no game files. Works alongside SDEncore.

### Downloads

Each release in [Releases](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/releases) has `SDTaxi.zip` (loader +
mod), `SDTaxi.asi` (the mod alone), `SDTaxi.pdb` (debug symbols) and `THIRD-PARTY-NOTICES.md`. Every commit on
`main` is built, tested and published as a prerelease `build-<N>` (not tested in game); builds verified in game
become full releases, which the README's download link points to.

### Building and testing

Visual Studio 2022 (v143), Windows SDK 10.0.26100. The project has to sit in the workspace's `mods\SDTaxi`, with
`reference\minhook` ([MinHook](https://github.com/TsudaKageyu/minhook) v1.3.4 sources, compiled in) next to it. From
the workspace root, `.\tools\build.ps1 -Mod SDTaxi -Test` builds and runs the tests: `load_test` loads the .asi
outside the game (it must not crash, must write its default ini and must report the game functions missing), and
`scan_test` checks the signature scan, including on a function whose start another hook has replaced. GitHub Actions
builds the same layout (`-warnAsError`), tests, packages and publishes prereleases; the dependencies are pinned in
`.github/reference.env` and `.github/asi-loader.env`.

### Credits

This mod uses or builds on the work of these people and projects. Thank you.

**Research**

- [SDmodding](https://github.com/SDmodding), almost all of it the work of one person, [sneakyevil](https://github.com/sneakyevil). This mod used:
  - the [Visual Studio 2022 project template](https://github.com/SDmodding/SDK/releases/tag/vs2022) released with SDmodding's [SDK](https://github.com/SDmodding/SDK): the mod's Visual Studio project derives from it, including its build settings and the source layout that starts at `dllmain.cc`;
  - the game's v1.0 exe and its debug symbols (PDB, shipped with the original Steam release), shared by
    SDmodding: the game's script system, the phone's contacts and how a taxi ride works were worked out from them;
  - the [SDK](https://github.com/SDmodding/SDK): the game's class names and data structures;
  - the property sets (where the taxi driver was found), action trees and symbol names (QSymbolsDictionary) exported in [Files](https://github.com/SDmodding/Files);
  - [BigFileSystem](https://github.com/SDmodding/BigFileSystem), [TheoryEngine](https://github.com/SDmodding/TheoryEngine), and the file name lists in sneakyevil's [SD-BigFileExplorer](https://github.com/sneakyevil/SD-BigFileExplorer) and in [Ekey](https://github.com/Ekey)'s
    SDDEUnpacker: the tool that reads the game's `.big` archives follows them; the game's scripts and action trees were taken out of the archives with it.

**The game's own content**

- The script that sends the taxi follows the game's own scripts for the Car Valet contact and for spawning taxis,
  written by United Front Games, © Square Enix. Everything from the hire prompt on (the fare, the map, skipping the
  trip) is the game's own taxi ride.
- SkookumScript (Agog Labs): the game's scripting language; the mod runs its script through the game's own
  compiler.

**Code in the mod** (full license texts in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md))

- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) (ThirteenAG): the `dinput8.dll` in the zip, which makes the game load mods.
  It contains MinHook, [miniz](https://github.com/richgel999/miniz) (Rich Geldreich and others) and [praydog](https://github.com/praydog)'s FunctionHookMinHook.
- [MinHook](https://github.com/TsudaKageyu/minhook) (Tsuda Kageyu, with Vyacheslav Patkov's Hacker Disassembler Engine): how the mod hooks into the game.

**Tools**

- [IDA Pro](https://hex-rays.com/ida-pro) (Hex-Rays) and [ida-pro-mcp](https://github.com/mrexodia/ida-pro-mcp) (mrexodia): analyzing the game's code.
- [Claude Code](https://claude.com/claude-code) (Anthropic): this mod was fully vibe-coded with Claude Fable and Opus; its code,
  documentation and reverse engineering are all Claude's, with little human review.

**The game and trademarks**

Sleeping Dogs: Definitive Edition was developed by United Front Games and published by Square Enix; the game
and its content are © Square Enix.

Not affiliated with Square Enix or United Front Games.
