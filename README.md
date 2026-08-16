# Codex Lite Light

将 Codex 的工作状态同步到 RP2040，并通过红、黄、绿三个 LED 展示状态。

本项目追求最小依赖：不扫描 Codex 日志、不读取内部数据库，也不安装 Codex
hooks。一个 Python 后台进程观察 Windows Terminal 标题开头的盲文转圈字符，
再通过 RP2040 的 USB 串口发送单字符状态命令。

## 项目结构

```text
.
├── host/       # 监听 Codex 窗口标题，并通过 USB 串口发送状态
├── firmware/   # RP2040 固件与硬件相关配置
├── docs/       # 通信协议、状态定义和接线说明
└── tools/      # 开发、烧录和调试辅助脚本
```

## 模块边界

- `host`：监听窗口标题中的工作动画，将聚合状态转换为单字符命令。
- `firmware`：通过 USB 串口接收命令，驱动三个 GPIO LED。
- `docs`：记录主机与 RP2040 之间的协议，避免两端实现相互耦合。

详细设计见 [docs/architecture.md](docs/architecture.md)、
[docs/hardware.md](docs/hardware.md) 和 [docs/protocol.md](docs/protocol.md)。

## 设计原则

- 状态来源只使用 Windows 窗口标题，不读取 Codex 私有数据。
- 多个 Codex 中任意一个处于工作状态时，状态灯都显示工作中。
- 主机与设备之间只使用 USB CDC 串口，不需要 BLE、Wi-Fi 或 HTTP 服务。
- 固件只负责串口解析、LED 映射和超时保护。
- 固件使用 GPIO9（红）、GPIO10（黄）、GPIO11（绿）。

## Run

依赖 Python 3 和 `pyserial`：

```powershell
pip install pyserial
./host/start-watcher.ps1 -Port COM5
```
