EESchema Schematic File Version 4
LIBS:power
LIBS:Device
LIBS:Connector_Generic
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Codex Lite Light"
Date "2026-08-22"
Rev "1.0"
Comp "Codex Lite Light"
Comment1 "RP2040-Zero with status LEDs and 1.54 inch ST7789 SPI TFT"
Comment2 "All GPIO signals are 3.3 V logic"
Comment3 "Firmware pin mapping: firmware/include/board_config.h"
Comment4 ""
$EndDescr
Text Notes 700 750 0    118  ~ 24
RP2040-Zero status LEDs
$Comp
L Connector_Generic:Conn_01x06 J1
U 1 1 66000001
P 1850 2250
F 0 "J1" H 1768 2667 50  0000 C CNN
F 1 "RP2040-Zero GPIO header" H 1768 2576 50 0000 C CNN
	1    1850 2250
	-1   0    0    -1
$EndComp
Text Label 2250 2050 0    50   ~ 0
3V3
Text Label 2250 2150 0    50   ~ 0
GND
Text Label 2250 2250 0    50   ~ 0
GPIO9_RED
Text Label 2250 2350 0    50   ~ 0
GPIO10_YELLOW
Text Label 2250 2450 0    50   ~ 0
GPIO11_GREEN
Text Label 2250 2550 0    50   ~ 0
USB_5V
Wire Wire Line
	1950 2050 2850 2050
Wire Wire Line
	1950 2150 2850 2150
Wire Wire Line
	1950 2250 2850 2250
Wire Wire Line
	1950 2350 2850 2350
Wire Wire Line
	1950 2450 2850 2450
Wire Wire Line
	1950 2550 2850 2550
$Comp
L Device:R R1
U 1 1 66000002
P 4100 2250
F 0 "R1" V 3893 2250 50 0000 C CNN
F 1 "330R" V 3984 2250 50 0000 C CNN
	1    4100 2250
	0    1    1    0
$EndComp
$Comp
L Device:LED D1
U 1 1 66000003
P 4700 2250
F 0 "D1" H 4693 1995 50 0000 C CNN
F 1 "RED" H 4693 2086 50 0000 C CNN
	1    4700 2250
	-1   0    0    1
$EndComp
Text Label 3500 2250 0    50   ~ 0
GPIO9_RED
Wire Wire Line
	3500 2250 3950 2250
Wire Wire Line
	4250 2250 4550 2250
Wire Wire Line
	4850 2250 5200 2250
Text Label 5050 2250 0    50   ~ 0
GND
$Comp
L Device:R R2
U 1 1 66000004
P 4100 2750
F 0 "R2" V 3893 2750 50 0000 C CNN
F 1 "330R" V 3984 2750 50 0000 C CNN
	1    4100 2750
	0    1    1    0
$EndComp
$Comp
L Device:LED D2
U 1 1 66000005
P 4700 2750
F 0 "D2" H 4693 2495 50 0000 C CNN
F 1 "YELLOW" H 4693 2586 50 0000 C CNN
	1    4700 2750
	-1   0    0    1
$EndComp
Text Label 3500 2750 0    50   ~ 0
GPIO10_YELLOW
Wire Wire Line
	3500 2750 3950 2750
Wire Wire Line
	4250 2750 4550 2750
Wire Wire Line
	4850 2750 5200 2750
Text Label 5050 2750 0    50   ~ 0
GND
$Comp
L Device:R R3
U 1 1 66000006
P 4100 3250
F 0 "R3" V 3893 3250 50 0000 C CNN
F 1 "330R" V 3984 3250 50 0000 C CNN
	1    4100 3250
	0    1    1    0
$EndComp
$Comp
L Device:LED D3
U 1 1 66000007
P 4700 3250
F 0 "D3" H 4693 2995 50 0000 C CNN
F 1 "GREEN" H 4693 3086 50 0000 C CNN
	1    4700 3250
	-1   0    0    1
$EndComp
Text Label 3500 3250 0    50   ~ 0
GPIO11_GREEN
Wire Wire Line
	3500 3250 3950 3250
Wire Wire Line
	4250 3250 4550 3250
Wire Wire Line
	4850 3250 5200 3250
Text Label 5050 3250 0    50   ~ 0
GND
Text Notes 3500 3550 0    50   ~ 0
LEDs are active-high. Use one resistor per LED; 330R is a starting value.
Text Notes 700 4100 0    118  ~ 24
1.54-inch ST7789 SPI TFT module
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 66000008
P 6450 5100
F 0 "J2" H 6530 5092 50 0000 L CNN
F 1 "ST7789 8-pin module" H 6530 5001 50 0000 L CNN
	1    6450 5100
	1    0    0    -1
$EndComp
Text Label 5650 4800 0    50   ~ 0
3V3
Text Label 5650 4900 0    50   ~ 0
GND
Text Label 5650 5000 0    50   ~ 0
GPIO14_SCK
Text Label 5650 5100 0    50   ~ 0
GPIO15_MOSI
Text Label 5650 5200 0    50   ~ 0
GPIO26_RES
Text Label 5650 5300 0    50   ~ 0
GPIO12_DC
Text Label 5650 5400 0    50   ~ 0
GPIO13_CS
Text Label 5650 5500 0    50   ~ 0
GPIO27_BLK
Wire Wire Line
	5650 4800 6250 4800
Wire Wire Line
	5650 4900 6250 4900
Wire Wire Line
	5650 5000 6250 5000
Wire Wire Line
	5650 5100 6250 5100
Wire Wire Line
	5650 5200 6250 5200
Wire Wire Line
	5650 5300 6250 5300
Wire Wire Line
	5650 5400 6250 5400
Wire Wire Line
	5650 5500 6250 5500
Text Notes 7050 4800 0    50   ~ 0
Pin order on TFT header:
Text Notes 7050 4925 0    50   ~ 0
1 VCC, 2 GND, 3 SCL, 4 SDA/MOSI
Text Notes 7050 5050 0    50   ~ 0
5 RES, 6 DC, 7 CS, 8 BLK
Text Notes 5650 5850 0    50   ~ 0
SDA is SPI MOSI (not I2C). No MISO connection is needed.
Text Notes 700 6800 0    50   ~ 0
RP2040-Zero native USB provides firmware serial communication. USB_5V is not connected to the TFT in this design.
$EndSCHEMATC
