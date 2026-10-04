# Sleeping Dogs: Definitive Edition — call a taxi from your phone (SDTaxi)

> [!IMPORTANT]
> **关于这个 mod**：它完全是用 Claude Code 里的 Claude Fable 和 Opus vibe coding 写出来的，几乎没有经过审查，请当作实验性质的 mod 使用，发现异常请反馈。作为一个长期缺少 mod 而自己下场做 mod 的普通玩家，我在 vibe 的过程中收获了许多快乐，并建议所有人都可以试着 vibe 一下去实现自己的灵感。由于这些代码都是 vibe 出来的，所以我不会以我的 mod 盈利，也不接受捐助。如果你喜欢我的作品，请考虑向[致谢](#致谢)中提到的组织和个人捐赠，祝你游玩愉快！
>
> **About this mod**: it was fully vibe-coded with Claude Fable and Opus in Claude Code, with little review, so treat
> it as experimental and please report anything unusual. I'm just an ordinary player who went a long time without
> mods for this game and finally started making them myself. Vibe coding them has been a lot of fun, and I'd encourage
> everyone to give it a try and bring their own ideas to life. Since all this code is vibe-coded, I won't make money
> from my mods and don't accept donations. If you like my work, please consider donating to the organizations and
> people listed in the [Credits](#credits) instead. Have fun!

[中文](#中文) | [English](#english)

## 中文

把《热血无赖：终极版》的手机变成叫车软件：通讯录里多一个 **Taxi** 联系人，打过去，附近就会派来一辆的士，开到你
身边停下等你。上车以后就是游戏原本的坐的士：付车费、在地图上选目的地，可以直接跳过路程。

状态：**早期版本**。在作者的电脑上测试正常。

> 适用于**任何版本**的《热血无赖：终极版》，Windows 10/11 64 位。想了解原理、自己编译或调参数，请看
> [ADVANCED.md](ADVANCED.md)。

### 安装（大约三分钟）

**第 1 步：下载**

点这里下载 **[SDTaxi.zip](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/releases/latest/download/SDTaxi.zip)**。

压缩包里只有这些：

```text
dinput8.dll                  ← Ultimate ASI Loader：让游戏加载 mod 的“加载器”
plugins\
    SDTaxi.asi                ← mod 本体
    SDTaxi-THIRD-PARTY-NOTICES.md
```

**第 2 步：打开游戏文件夹**

1. 打开 Steam，进入「库」。
2. 在左侧列表里右键点「Sleeping Dogs: Definitive Edition」→「管理」→「浏览本地文件」。
3. 弹出来的就是游戏文件夹，里面有 `sdhdship.exe`（如果电脑不显示扩展名，就是一个叫 `sdhdship` 的程序）。

**第 3 步：把文件放进去**

1. 双击打开下载的 `SDTaxi.zip`。
2. 选中里面的 `dinput8.dll` 和 `plugins` 文件夹，一起拖进游戏文件夹。
3. 如果 Windows 弹出「替换或跳过文件」，说明游戏文件夹里已经有 `dinput8.dll` 了（你以前装过别的 mod，
   加载器已经在了），选「跳过该文件」。已有的 `plugins` 文件夹会自动合并，不用管。

放好后，游戏文件夹里应该是这样（只列出相关的部分）：

```text
SleepingDogsDefinitiveEdition\
    sdhdship.exe
    dinput8.dll
    plugins\
        SDTaxi.asi
```

注意 `dinput8.dll` 要和 `sdhdship.exe` 在同一层，不要多套一层文件夹。

**第 4 步：启动游戏**

照常从 Steam 启动游戏。`plugins` 里多出 `SDTaxi.ini` 和 `SDTaxi.log` 两个文件，就说明 mod 已经加载。

### 怎么用

1. 在游戏里打开手机 →「通讯录」，往下翻到 **Taxi**，选中拨打。
2. 电话挂断后，一辆的士从附近开过来，在你身边停下等你，地图上有它的标记。
3. 走到车旁，像在街上拦的士一样按住屏幕提示的键叫车，付车费上车。
4. 在地图上选目的地并确认，画面一黑，车就到了目的地附近。

的士在路上时，这个联系人会暂时从通讯录里消失，坐完车后再出现。叫来的车两分钟内没人上，就会自己开走。

### 常见问题

**想换成绿色的新界的士，或者改联系人的名字**

用记事本打开 `plugins\SDTaxi.ini`，每一项都有中文说明。例如把 `[Taxi]` 下的 `Vehicle` 改成
`object-physical-vehicle-625MHCTaxiGreen01` 就是绿色的士。保存后重启游戏。

**打电话后提示 “No taxi can get here.”**

附近没有车能开过来的路。走到大路边上再打一次。

**`plugins` 里没有 `SDTaxi.log`**

说明 mod 没被加载：检查 `dinput8.dll` 是否和 `sdhdship.exe` 在同一层，杀毒软件有没有删掉它（ASI 加载器偶尔
会被误报，可以从隔离区还原并把游戏文件夹加入排除项）。如果第 3 步跳过了原有的 `dinput8.dll`，那个文件可能
不是 ASI 加载器，备份后换成压缩包里的。

**更新**

下载新的 `SDTaxi.zip`，只把里面的 `plugins` 文件夹拖进游戏文件夹，Windows 询问时选「替换目标中的文件」。
`SDTaxi.ini` 不在压缩包里，你的设置会保留。

**卸载**

删掉 `plugins` 里的 `SDTaxi.asi`、`SDTaxi.ini` 和 `SDTaxi.log`。如果 `plugins` 里已经没有其他 `.asi` 文件了，
`dinput8.dll` 也可以删掉。

**遇到问题怎么反馈**

在 [GitHub Issues](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/issues) 里说明情况，并附上
`plugins\SDTaxi.log`。

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

Turns the phone in Sleeping Dogs: Definitive Edition into a ride-hailing app: a new **Taxi** contact. Call it and a
cab nearby is sent to you; it pulls up next to you and waits. Once you're in, it's the game's own taxi ride: pay the
fare, pick a destination on the map, and skip the trip if you like.

Status: **early**. Works on the author's PC.

> Works with **any version** of Sleeping Dogs: Definitive Edition, Windows 10/11 64-bit. How it works, building
> the mod and all settings: [ADVANCED.md](ADVANCED.md).

### Install (about three minutes)

**Step 1: download**

Download **[SDTaxi.zip](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/releases/latest/download/SDTaxi.zip)**.

It only contains:

```text
dinput8.dll                  ← Ultimate ASI Loader: what makes the game load mods
plugins\
    SDTaxi.asi                ← the mod
    SDTaxi-THIRD-PARTY-NOTICES.md
```

**Step 2: open the game folder**

1. Open Steam and go to your Library.
2. Right-click "Sleeping Dogs: Definitive Edition" → Manage → Browse local files.
3. That's the game folder; it contains `sdhdship.exe` (or `sdhdship`, if file extensions are hidden).

**Step 3: put the files in**

1. Open the downloaded `SDTaxi.zip`.
2. Select `dinput8.dll` and the `plugins` folder and drag both into the game folder.
3. If Windows asks whether to replace or skip a file, the game folder already has a `dinput8.dll` (you've
   installed a mod before and the loader is there): choose "Skip this file". An existing `plugins` folder is
   merged automatically.

Afterwards the game folder should look like this (only the relevant part):

```text
SleepingDogsDefinitiveEdition\
    sdhdship.exe
    dinput8.dll
    plugins\
        SDTaxi.asi
```

`dinput8.dll` has to be next to `sdhdship.exe`, not in a subfolder.

**Step 4: start the game**

Start the game from Steam as usual. When `SDTaxi.ini` and `SDTaxi.log` appear in `plugins`, the mod is loaded.

### How to use

1. In the game, open the phone → Contacts, scroll down to **Taxi** and call it.
2. After the call, a cab drives over from nearby and stops next to you to wait; it's marked on the map.
3. Walk up to it and hold the button the prompt shows to hire it, as you would with a cab on the street; pay the
   fare and get in.
4. Pick a destination on the map and confirm: after a short fade the cab is nearly there.

While the cab is on its way, the contact is hidden; it's back after the ride. A cab nobody gets into within two
minutes drives off.

### FAQ

**I want the green New Territories taxi, or a different name for the contact**

Open `plugins\SDTaxi.ini` in Notepad; every setting is explained in the file. For example
`Vehicle = object-physical-vehicle-625MHCTaxiGreen01` under `[Taxi]` sends the green taxi. Save and restart the game.

**After the call it says "No taxi can get here."**

There's no road nearby a car could come by. Walk over to a main road and call again.

**There's no `SDTaxi.log` in `plugins`**

The mod wasn't loaded: check that `dinput8.dll` is next to `sdhdship.exe` and that your antivirus didn't remove
it (ASI loaders are sometimes flagged; restore it from quarantine and exclude the game folder). If you skipped
an existing `dinput8.dll` in step 3, that file may not be an ASI loader: back it up and use the one from the zip.

**Updating**

Download the new `SDTaxi.zip` and drag only its `plugins` folder into the game folder; choose "Replace the files
in the destination". `SDTaxi.ini` isn't in the zip, so your settings stay.

**Uninstalling**

Delete `SDTaxi.asi`, `SDTaxi.ini` and `SDTaxi.log` from `plugins`. If there are no other `.asi` files left in
`plugins`, you can delete `dinput8.dll` too.

**Reporting a problem**

Describe it in [GitHub Issues](https://github.com/aUsernameWoW/sleeping-dogs-e-hailing/issues) and attach
`plugins\SDTaxi.log`.

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
