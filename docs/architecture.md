# Architecture

## Data flow

```text
Windows visible window titles
          |
          | poll for leading Braille spinner (U+2800..U+28FF)
          v
Python title watcher (one background process)
          |
          | aggregate state + USB serial command
          v
RP2040-Zero firmware -> red / yellow / green GPIO LEDs + ST7789 TFT
```

项目不读取 `history.jsonl`、SQLite、Codex 日志或对话内容，也不安装 lifecycle
hooks。Windows Terminal 中的 Codex 正在工作时，会在标题开头显示一个不断变化的
盲文字符，例如 `⠋ codex-lite-light`。后台监听器只判断标题首字符是否位于
Unicode 盲文区间 `U+2800..U+28FF`。

## State detection

监听器每 100 ms 枚举一次可见窗口：

| Observed condition | Serial command | Result |
| --- | --- | --- |
| At least one matching title has a spinner | `W` | 黄灯闪烁 |
| Working changes to no spinner | `S` | 绿灯常亮 |
| Starts while no spinner exists | `I` | 绿灯常亮 |

监听器每 20 秒重发当前状态作为心跳，避免固件的 60 秒链路超时使灯熄灭。它作为
唯一进程持续持有串口，其他串口助手和 Serial Monitor 不应同时打开同一 COM 口。

## Multiple Codex terminals

多个 Codex 实例使用 OR 聚合：

```text
working = terminal_1_working OR terminal_2_working OR ...
```

只要任意一个标题仍有转圈字符，状态就是 `WORKING`。只有所有 Codex 都停止转圈，
状态才切换为 `SUCCESS`。当前灯光不区分具体实例，也不显示活动实例数量。

## LED behavior

| State | Red | Yellow | Green |
| --- | --- | --- | --- |
| `OFF` | off | off | off |
| `IDLE` | off | off | steady |
| `WORKING` | off | blink | off |
| `WAITING` | blink | steady | off |
| `SUCCESS` | off | off | steady |
| `ERROR` | steady | off | off |

标题监听器目前只自动产生 `IDLE`、`WORKING` 和 `SUCCESS`。`WAITING` 与 `ERROR`
仍可通过串口手动发送，但仅凭标题无法可靠判断这两个状态。

## TFT behavior

| State | TFT output |
| --- | --- |
| `OFF` | black |
| `IDLE` / `SUCCESS` | green |
| `WORKING` | rotating OpenAI Blossom on black |
| `WAITING` | orange |
| `ERROR` | red |

`WORKING` 动画不是预渲染帧。固件由官方 Blossom 轮廓生成 84x84 的 1-bit 掩码，
运行时计算旋转、薄型挤出、深度遮挡和明暗，并把 128x128 RGB565 帧居中写入
240x240 屏幕。目标刷新间隔为 33 ms；实际帧率还取决于 RP2040 的计算时间和
24 MHz SPI1 传输时间。

动画缓冲区和深度缓冲区是主要 RAM 开销。它们使用静态分配，避免动画过程中发生
堆分配或内存碎片。官方轮廓的 PNG/SVG 源文件位于 `firmware/assets`，压缩后的固件
掩码由 `tools/generate_blossom_mask.py` 生成。

## Limitations

- 仅支持 Windows 可见窗口标题；最小化窗口仍可检测，已关闭的窗口不可检测。
- 任何标题以盲文字符开头的可见程序都可能被识别为工作中。
- 标题格式若随 Codex/Windows Terminal 更新而变化，需要同步调整检测规则。
- BLE、Wi-Fi、HTTP relay、远程状态和每实例灯光均不在当前范围内。
