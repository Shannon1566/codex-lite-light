# RP2040 firmware

固件通过 RP2040 USB 串口接收状态命令，并控制红、黄、绿三个 GPIO。项目使用
PlatformIO 当前 `pico` 板定义支持的 Arduino framework 构建。

固件职责保持简单：

- 初始化三个 GPIO 和 USB CDC；
- 按行读取单字符命令；
- 生成常亮或闪烁效果；
- 收到未知命令时保持原状态；
- 长时间未收到状态时进入安全的 `OFF` 状态。

默认引脚：

| LED | RP2040 GPIO |
| --- | --- |
| Green | GPIO11 |
| Yellow | GPIO10 |
| Red | GPIO9 |

这些编号和 LED 高/低电平有效方式应集中在一个板级配置文件中，不散落在业务
代码里。默认按高电平点亮处理。

## Build

使用 PlatformIO Core：

```powershell
pio run -d firmware
```

生成的 UF2 位于 `firmware/.pio/build/pico/firmware.uf2`。按住 RP2040 开发板的
BOOTSEL 键连接 USB，将 UF2 复制到出现的 RPI-RP2 磁盘即可烧录。
