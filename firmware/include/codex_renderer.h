#ifndef CODEX_LIGHT_CODEX_RENDERER_H
#define CODEX_LIGHT_CODEX_RENDERER_H

#include <Arduino.h>

constexpr uint16_t CODEX_RENDER_SIZE = 128;

void renderCodexFrame(uint16_t* framebuffer, float rotationRadians);

#endif
