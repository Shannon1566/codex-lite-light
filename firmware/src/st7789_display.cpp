#include "st7789_display.h"

#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "board_config.h"

namespace {

constexpr uint8_t SleepOut = 0x11;
constexpr uint8_t InversionOn = 0x21;
constexpr uint8_t DisplayOn = 0x29;
constexpr uint8_t ColumnAddressSet = 0x2A;
constexpr uint8_t RowAddressSet = 0x2B;
constexpr uint8_t MemoryWrite = 0x2C;
constexpr uint8_t MemoryAccessControl = 0x36;
constexpr uint8_t PixelFormatSet = 0x3A;

void writeByte(uint8_t value) {
    spi_write_blocking(spi1, &value, 1);
}

void selectDisplay() {
    digitalWrite(CODEX_LIGHT_TFT_CS, LOW);
}

void deselectDisplay() {
    digitalWrite(CODEX_LIGHT_TFT_CS, HIGH);
}

}  // namespace

void St7789Display::writeCommand(uint8_t command) {
    digitalWrite(CODEX_LIGHT_TFT_DC, LOW);
    selectDisplay();
    writeByte(command);
    deselectDisplay();
    digitalWrite(CODEX_LIGHT_TFT_DC, HIGH);
}

void St7789Display::writeData(uint8_t data) {
    writeData(&data, 1);
}

void St7789Display::writeData(const uint8_t* data, size_t length) {
    digitalWrite(CODEX_LIGHT_TFT_DC, HIGH);
    selectDisplay();
    while (length-- > 0) {
        writeByte(*data++);
    }
    deselectDisplay();
}

void St7789Display::begin() {
    pinMode(CODEX_LIGHT_TFT_CS, OUTPUT);
    pinMode(CODEX_LIGHT_TFT_DC, OUTPUT);
    pinMode(CODEX_LIGHT_TFT_RESET, OUTPUT);
    pinMode(CODEX_LIGHT_TFT_BACKLIGHT, OUTPUT);
    digitalWrite(CODEX_LIGHT_TFT_CS, HIGH);
    digitalWrite(CODEX_LIGHT_TFT_DC, HIGH);
    digitalWrite(CODEX_LIGHT_TFT_RESET, HIGH);
    digitalWrite(CODEX_LIGHT_TFT_BACKLIGHT, HIGH);

    _spi_init(spi1, CODEX_LIGHT_TFT_SPI_HZ);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(CODEX_LIGHT_TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(CODEX_LIGHT_TFT_MOSI, GPIO_FUNC_SPI);

    digitalWrite(CODEX_LIGHT_TFT_RESET, LOW);
    delay(100);
    digitalWrite(CODEX_LIGHT_TFT_RESET, HIGH);
    delay(100);
    digitalWrite(CODEX_LIGHT_TFT_BACKLIGHT, HIGH);
    delay(100);

    writeCommand(SleepOut);
    delay(120);

    // Vendor sequence for ZJY154T-PG04, portrait orientation, RGB565.
    writeCommand(MemoryAccessControl);
    writeData(0x00);
    writeCommand(PixelFormatSet);
    writeData(0x05);

    const uint8_t porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    writeCommand(0xB2);
    writeData(porch, sizeof(porch));
    writeCommand(0xB7);
    writeData(0x35);
    writeCommand(0xBB);
    writeData(0x32);
    writeCommand(0xC2);
    writeData(0x01);
    writeCommand(0xC3);
    writeData(0x15);
    writeCommand(0xC4);
    writeData(0x20);
    writeCommand(0xC6);
    writeData(0x0F);
    const uint8_t power[] = {0xA4, 0xA1};
    writeCommand(0xD0);
    writeData(power, sizeof(power));

    const uint8_t positiveGamma[] = {
        0xD0, 0x08, 0x0E, 0x09, 0x09, 0x05, 0x31,
        0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34,
    };
    const uint8_t negativeGamma[] = {
        0xD0, 0x08, 0x0E, 0x09, 0x09, 0x15, 0x31,
        0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34,
    };
    writeCommand(0xE0);
    writeData(positiveGamma, sizeof(positiveGamma));
    writeCommand(0xE1);
    writeData(negativeGamma, sizeof(negativeGamma));

    writeCommand(InversionOn);
    writeCommand(DisplayOn);
}

void St7789Display::setAddressWindow(uint16_t x, uint16_t y, uint16_t width,
                                     uint16_t height) {
    const uint16_t xEnd = x + width - 1u;
    const uint16_t yEnd = y + height - 1u;
    const uint8_t columns[] = {
        static_cast<uint8_t>(x >> 8u), static_cast<uint8_t>(x),
        static_cast<uint8_t>(xEnd >> 8u), static_cast<uint8_t>(xEnd),
    };
    const uint8_t rows[] = {
        static_cast<uint8_t>(y >> 8u), static_cast<uint8_t>(y),
        static_cast<uint8_t>(yEnd >> 8u), static_cast<uint8_t>(yEnd),
    };
    writeCommand(ColumnAddressSet);
    writeData(columns, sizeof(columns));
    writeCommand(RowAddressSet);
    writeData(rows, sizeof(rows));
    writeCommand(MemoryWrite);
}

void St7789Display::fillRect(uint16_t x, uint16_t y, uint16_t width,
                             uint16_t height, uint16_t color) {
    if (x >= Width || y >= Height || width == 0 || height == 0) return;
    width = min<uint16_t>(width, Width - x);
    height = min<uint16_t>(height, Height - y);
    setAddressWindow(x, y, width, height);

    const uint8_t high = static_cast<uint8_t>(color >> 8u);
    const uint8_t low = static_cast<uint8_t>(color);
    uint32_t pixels = static_cast<uint32_t>(width) * height;
    digitalWrite(CODEX_LIGHT_TFT_DC, HIGH);
    selectDisplay();
    while (pixels-- > 0) {
        writeByte(high);
        writeByte(low);
    }
    deselectDisplay();
}

void St7789Display::fillScreen(uint16_t color) {
    fillRect(0, 0, Width, Height, color);
}

void St7789Display::drawRgb565(uint16_t x, uint16_t y, uint16_t width,
                               uint16_t height, const uint16_t* pixels) {
    if (x >= Width || y >= Height || width == 0 || height == 0 || pixels == nullptr) {
        return;
    }
    width = min<uint16_t>(width, Width - x);
    height = min<uint16_t>(height, Height - y);
    setAddressWindow(x, y, width, height);

    uint32_t count = static_cast<uint32_t>(width) * height;
    digitalWrite(CODEX_LIGHT_TFT_DC, HIGH);
    selectDisplay();
    uint8_t transferBuffer[256];
    while (count > 0) {
        const uint16_t batch = min<uint32_t>(count, sizeof(transferBuffer) / 2u);
        for (uint16_t i = 0; i < batch; ++i) {
            const uint16_t color = *pixels++;
            transferBuffer[i * 2u] = static_cast<uint8_t>(color >> 8u);
            transferBuffer[i * 2u + 1u] = static_cast<uint8_t>(color);
        }
        spi_write_blocking(spi1, transferBuffer, batch * 2u);
        count -= batch;
    }
    deselectDisplay();
}

void St7789Display::showSelfTest() {
    fillRect(0, 0, Width, 60, rgb565(255, 0, 0));
    fillRect(0, 60, Width, 60, rgb565(0, 255, 0));
    fillRect(0, 120, Width, 60, rgb565(0, 0, 255));
    fillRect(0, 180, Width, 60, rgb565(255, 255, 255));
}
