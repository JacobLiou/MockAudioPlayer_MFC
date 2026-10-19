# AGENTS.md

面向 AI 编码代理与新成员的项目约定。总原则:**简单模仿微软 MFC 最佳实践,不引入额外复杂度。**

## 项目概述

- Windows 桌面 MFC 对话框应用 `MockMusicPlayer`("模拟音乐播放器")。
- 界面(布局、控件)已完成,业务逻辑在 `CMockMusicPlayerDlg` 中逐步补充。
- Visual Studio 2022 / v143 / Unicode / 动态链接 MFC / 预编译头 `pch.h`。

## 构建与验证

- 在 *Developer Command Prompt for VS 2022* 中执行(无特殊说明一律构建 Win32 配置):

  ```bat
  msbuild MockMusicPlayer.sln /p:Configuration=Debug /p:Platform=Win32
  ```

- 本项目**没有自动化测试**。提交前的验证标准:
  1. 编译零错误、零警告(工程为 `/W3` + SDL 检查);
  2. 程序可启动,所改控件行为正常(手动冒烟)。
- 改完必须本地编译一次再提交。

## 代码规范(微软最佳实践)

### 命名

- 类:`C` 前缀 + 大驼峰(`CMockMusicPlayerDlg`);函数、变量用大驼峰。
- 成员变量:`m_` 前缀(`m_hIcon`),与 MFC 向导生成风格一致。
- 消息处理函数:`OnBnClickedBtnXxx`、`OnLbnSelchangeListXxx` 等 MFC 向导风格命名。
- 控件资源 ID:`IDC_BTN_*` / `IDC_LIST_*` / `IDC_SLIDER_*` / `IDC_CHECK_*` / `IDC_STATIC_*`,统一定义在 `resource.h`,沿用既有编号段(控件从 1000 递增)。

### 字符串与编码(重要)

- 工程 Unicode 字符集:字符串字面量一律 `_T("...")`,优先用 `CString`,不要混用窄字符 API。
- 源码 `.h/.cpp` 保持 **UTF-8 with BOM**(VS 默认)。
- `.rc` 资源文件是 **GBK(代码页 936)** 编码:严禁以 UTF-8 重写整个 `.rc`,否则中文资源全部乱码。尽量用 VS 资源编辑器改资源;`.aps` 是资源编辑器缓存,禁止手工编辑或提交手工改动。

### MFC 惯用法

- 控件事件走消息映射:`BEGIN_MESSAGE_MAP` + `ON_BN_CLICKED` 等标准宏,不要用 `ON_MESSAGE` 自造轮子。
- 控件访问:简单场景 `GetDlgItem`,频繁访问的控件用 `DDX_Control` 绑定成员。
- 优先使用 MFC 工具类(`CString`、`CStringArray`、`CFileDialog` 等),风格与向导生成代码保持一致。
- 系统资源(图标、GDI 句柄、MCI 设备)遵循 RAII,确保成对打开/关闭,不得泄漏。
- 错误提示用 `AfxMessageBox`,不要自造弹窗。

### 现代 C++

- 允许 C++14/17 特性,但以可读、简单为先;小项目,不过度抽象。

## 架构约定

- 一切从简:目前允许直接把逻辑写在 `CMockMusicPlayerDlg` 中,不提前分层。
- 功能变大时再拆独立类(如 `CPlaylistMgr`、`CPlayerEngine`),一对 `.h/.cpp`,命名与目录随现有结构。
- 新增 `.cpp` 第一行必须是 `#include "pch.h"`(PCH);新建源文件需同步登记到 `.vcxproj` 与 `.vcxproj.filters`。

## 播放引擎约定(硬性)

- **就近采用 Windows 自带 API / Win32 API**:首选 MCI(`mciSendString`,支持 mp3/wav 等,最简),备选 `waveOut`、WASAPI。
- **禁止引入任何第三方多媒体库**(FMOD、BASS、SDL_mixer、miniaudio 等)及任何 NuGet / vcpkg 依赖。`doc/logo-fmod.svg` 仅为参考项目遗留素材,不要据此选型。
- MCI 设备遵循"打开→播放→关闭"成对调用;窗口关闭前必须释放。

## 禁止事项

- 禁止改动 `.vcxproj` 的平台工具集、字符集、MFC 使用方式、PCH 配置等全局设置。
- 禁止重排或复用 `resource.h` 既有 ID(代码已引用);新增 ID 顺延 `_APS_NEXT_CONTROL_VALUE`。
- 禁止把构建产物(`Debug/`、`x64/`、`.aps`、`.vs/`)提交入库。
