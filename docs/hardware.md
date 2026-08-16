# Hardware

## Components

- 一块带 USB 接口的 RP2040 开发板；
- 红、黄、绿 LED 各一个；
- 每个 LED 一个限流电阻，建议从 220 ohm 到 1 kohm 之间选择；
- USB 数据线。

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
