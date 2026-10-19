# MockMusicPlayer · 一个小小的 MFC 音乐播放器

一个基于 MFC 的 Windows 桌面音乐播放器。界面参照常见播放器布局"简单地模仿"实现,
**不引入任何第三方多媒体库**,播放引擎就近采用 Windows 自带 API / Win32 多媒体接口,
开箱即编译、零外部依赖。

![主界面](doc/main-interface.png)

## 功能一览

**播放列表**
- 添加 / 移除 / 全部移除曲目
- 加载 / 保存列表文件
- A–Z / Z–A 排序,上移 / 下移调整顺序

**播放控制**
- 播放 / `||` 暂停 / `■` 停止
- `|<` 上一首 / `>|` 下一首,`<<` 快退 / `>>` 快进
- 进度滑块与时间显示(`00:00 / 00:00`)

**音量**
- 音量滑块(0% – 100%)

**播放选项**
- 循环播放 / 自动下一首 / 随机播放

## 技术栈

| 项 | 说明 |
| --- | --- |
| 框架 | MFC 对话框应用(`CDialogEx`) |
| 语言 | C++(Visual Studio 2022,v143 工具集) |
| 字符集 | Unicode |
| 播放引擎 | Windows 自带 MCI 多媒体接口(`mciSendString`,mpegvideo 设备,零第三方依赖) |
| 平台 | Windows 10 / 11(Win32 x86,vcxproj 亦内置 x64 配置) |

## 环境要求

- Visual Studio 2022
  - 工作负载:**使用 C++ 的桌面开发**
  - 单个组件:**适用于 v143 生成工具的 MFC(x86 和 x64)**
- Windows 10 / 11

## 构建与运行

1. 打开 `MockMusicPlayer.sln`。
2. 选择 `Debug` 或 `Release` × `Win32` 配置,`Ctrl+F5` 启动调试或运行。

命令行构建(在 *Developer Command Prompt for VS 2022* 中):

```bat
msbuild MockMusicPlayer.sln /p:Configuration=Release /p:Platform=Win32
```

## 项目结构

```
MockMusicPlayer.sln
├─ MockMusicPlayer/
│  ├─ MockMusicPlayer.h/.cpp       # CWinApp 应用类
│  ├─ MockMusicPlayerDlg.h/.cpp    # 主对话框:界面 + 播放列表/播放逻辑
│  ├─ MciPlayer.h/.cpp             # MCI 播放引擎(mciSendString 封装)
│  ├─ resource.h                    # 资源 ID 定义(IDC_BTN_* 等)
│  ├─ MockMusicPlayer.rc            # 对话框/图标/位图资源(GBK 编码)
│  ├─ pch.h / pch.cpp               # 预编译头
│  ├─ framework.h / targetver.h     # 标准 包含
│  └─ res/                          # 图标、位图等资源文件
└─ doc/                            # 截图与设计资料(UML 类图等)
```

## 路线图

- [x] 界面布局与全部控件(播放列表 / 控制 / 音量 / 选项)
- [x] 播放列表管理逻辑(增删、排序、上下移、m3u 列表加载/保存)
- [x] 基于 Windows MCI 的播放引擎(打开/播放/暂停/定位/音量/时长查询)
- [x] 进度 / 音量滑块与播放状态联动
- [x] 循环、自动下一首、随机播放策略

## 许可证

[GPL-3.0](LICENSE)
