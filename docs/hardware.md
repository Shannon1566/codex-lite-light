# Hardware

## Components

- 一块带 USB 接口的 RP2040 开发板；
- 红、黄、绿 LED 各一个；
- 每个 LED 一个限流电阻，建议从 220 ohm 到 1 kohm 之间选择；
- USB 数据线。
- 一块 1.54 英寸、240x240、ST7789、8 针 SPI TFT 模块。

## Wiring

每个 LED 使用一个独立 GPIO：

```text
RP2040 GPIO9  -> resistor -> red LED    -> GND
RP2040 GPIO10 -> resistor -> yellow LED -> GND
RP2040 GPIO11 -> resistor -> green LED  -> GND
```

| LED | GPIO | Default active level |
| --- | --- | --- |
| Green | GPIO11 | High |
| Yellow | GPIO10 | High |
| Red | GPIO9 | High |

默认设计按共阴极/单 LED、高电平点亮处理。若使用共阳极红绿灯模块，只需在板级
配置中启用低电平有效，并将公共端连接到 3.3 V。

根据硬件实测，本项目固定使用 GPIO9（红）、GPIO10（黄）、GPIO11（绿），不照搬
参考 ESP32-C3 项目的 IO2、IO3、IO4。

## ST7789 TFT wiring

屏幕模块按资料中的 8 针接口接线。所有信号均为 3.3 V 逻辑，`SDA` 在这个只写
SPI 接口中是 MOSI，不是 I2C SDA；模块没有需要连接的 MISO。

| TFT module | RP2040-Zero | Purpose |
| --- | --- | --- |
| GND | GND | Common ground |
| VCC | 3V3(OUT) | 3.3 V supply |
| SCL | GPIO14 | SPI1 clock |
| SDA | GPIO15 | SPI1 MOSI/data |
| RES | GPIO26 | Active-low reset |
| DC | GPIO12 | Data/command select |
| CS | GPIO13 | Active-low chip select |
| BLK | GPIO27 | Backlight enable, high = on |

不要把 VCC 接到 Pico 的 `VBUS`/5 V。资料中的裸屏背光额定约 3.0 V、60 mA；本项目
针对资料所示带 S8050 背光开关和限流电阻的 8 针转接板。如果手中是 12 针裸 FPC
屏，不能按上表直接接 Pico，需使用恒流背光驱动和合适的 FPC 转接电路。

GPIO14 是 RP2040 的 SPI1 SCK，GPIO15 是 SPI1 TX/MOSI。RP2040-Zero 板边没有
GPIO16--25 的普通排针，因此复位和背光使用板上引出的 GPIO26、GPIO27。

## Bring-up and troubleshooting

1. 断电后先用万用表通断档逐根检查跳线；导线端到端应接近 0 ohm，不能是兆欧级。
2. 烧录后屏幕应短暂显示红、绿、蓝、白四条自检色带。
3. 只有背光、没有画面时，优先检查 `SCL`、`SDA`、`CS`、`DC` 和 `RES` 的连续性，
   特别是面包板跳线与屏幕排针之间是否真正导通。
4. `SCL` 工作时万用表可能只能显示跳变后的平均电压，不能用这个读数判断 SPI
   波形是否正确；但固定为 0 V 或始终不导通通常说明接线问题。
5. USB 串口和 TFT 使用不同外设，串口本身不会占用 GPIO14/15，也不会导致黑屏。

屏幕驱动采用 SPI mode 0、MSB first、RGB565，默认时钟为 24 MHz。若长线导致花屏，
可在 `firmware/include/board_config.h` 中降低 `CODEX_LIGHT_TFT_SPI_HZ`。
