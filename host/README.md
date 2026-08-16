# Host title watcher

Windows 主机端使用单个 Python 后台进程监听 Codex 的 Windows Terminal 标题。
它不读取 Codex 日志、数据库或对话内容，也不依赖 Codex lifecycle hooks。

## Requirements

- Windows 10/11
- Python 3.10+
- `pyserial`
- RP2040 USB 串口（当前默认为 `COM5`）

```powershell
pip install pyserial
```

## Start

```powershell
./host/start-watcher.ps1 -Port COM5
```

[codex_title_watcher.py](codex_title_watcher.py) 每 100 ms 枚举可见窗口。标题首字符
属于 Unicode 盲文区间时，该窗口被视为工作中。任意一个窗口工作时发送 `W`；
所有窗口停止后发送 `S`。

监听器具有以下保护：

- Windows 命名互斥锁，阻止重复启动；
- 持续持有一个串口连接，避免多进程竞争；
- 串口断开后自动重连；
- 每 20 秒发送一次状态心跳；
- 日志轮转，文件为 `host/codex-light-watcher.log`。

可先检查当前标题检测结果，不打开串口：

```powershell
python ./host/codex_title_watcher.py --once
```

输出 `W` 表示至少一个 Codex 正在工作，输出 `I` 表示没有检测到转圈标题。

## Multiple terminals

多个 Codex 窗口采用 OR 聚合。任意一个窗口工作时保持黄灯闪烁，所有窗口都完成
后才切换绿灯。监听器不区分窗口身份或任务数量。

## Legacy files

`codex-light.ps1`、`test-codex-light.ps1` 和 `codex-hooks.toml.example` 是早期 hook
方案的遗留文件，不属于当前推荐运行路径。用户级 `~/.codex/hooks.json` 不应安装。
