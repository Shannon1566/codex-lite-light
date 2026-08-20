#ifndef CODEX_LIGHT_ST7789_DISPLAY_H
#define CODEX_LIGHT_ST7789_DISPLAY_H

#include <Arduino.h>

class St7789Display {
public:
    static constexpr uint16_t Width = 240;
    static constexpr uint16_t Height = 240;

    void begin();
    void fillScreen(uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                  uint16_t color);
    void drawRgb565(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                    const uint16_t* pixels);
    void showSelfTest();

    static constexpr uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
        return static_cast<uint16_t>(((red & 0xF8u) << 8u) |
                                     ((green & 0xFCu) << 3u) |
                                     (blue >> 3u));
    }

private:
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);
    void writeData(const uint8_t* data, size_t length);
    void setAddressWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
};

#endif
