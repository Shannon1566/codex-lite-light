# Serial protocol

## Transport

- RP2040 原生 USB CDC 串口；
- 默认 `115200 8N1`（USB CDC 实际传输不依赖波特率，但保留常规配置）；
- UTF-8/ASCII 文本；
- 每条命令以 LF 结尾，接收端同时容忍 CRLF。

## Commands

| Line | State |
| --- | --- |
| `O` | `OFF` |
| `I` | `IDLE` |
| `W` | `WORKING` |
| `A` | `WAITING` (approval required) |
| `S` | `SUCCESS` |
| `E` | `ERROR` |
| `?` | 查询当前状态 |

设备收到查询后返回：

```text
STATE <command>\n
```

例如：

```text
W\n
STATE W\n
```

使用单字符行协议便于在任意平台调试，也避免在 RP2040 上引入 JSON 解析器。未知
命令返回 `ERR UNKNOWN`，且不改变当前灯光状态。
