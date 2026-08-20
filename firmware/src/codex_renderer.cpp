#include "codex_renderer.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "openai_blossom_mask.h"

namespace {

constexpr int16_t ModelSize = OPENAI_BLOSSOM_MASK_SIZE;
constexpr int16_t HalfModel = ModelSize / 2;
constexpr int8_t HalfThickness = 4;
constexpr int16_t FixedOne = 16384;

int16_t depthBuffer[CODEX_RENDER_SIZE * CODEX_RENDER_SIZE];

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    return static_cast<uint16_t>(((red & 0xF8u) << 8u) |
                                 ((green & 0xFCu) << 3u) |
                                 (blue >> 3u));
}

bool sourceMaskAt(int16_t x, int16_t y) {
    if (x < 0 || x >= ModelSize || y < 0 || y >= ModelSize) return false;
    const uint16_t bitIndex = static_cast<uint16_t>(y) * ModelSize + x;
    return (OPENAI_BLOSSOM_MASK[bitIndex >> 3u] & (1u << (bitIndex & 7u))) != 0;
}

bool maskAt(int16_t x, int16_t y) {
    // A one-pixel dilation gives the small display the visual weight of the
    // official mark without changing its interlocking geometry.
    return sourceMaskAt(x, y) || sourceMaskAt(x - 1, y) || sourceMaskAt(x + 1, y) ||
           sourceMaskAt(x, y - 1) || sourceMaskAt(x, y + 1);
}

void plotProjected(uint16_t* framebuffer, int16_t modelX, int16_t modelY, int8_t modelZ,
                   int16_t sine, int16_t cosine, uint16_t color, uint8_t splat) {
    const int32_t rotatedX = static_cast<int32_t>(modelX) * cosine +
                             static_cast<int32_t>(modelZ) * sine;
    const int32_t rotatedZ = -static_cast<int32_t>(modelX) * sine +
                             static_cast<int32_t>(modelZ) * cosine;
    const int16_t screenX = static_cast<int16_t>(CODEX_RENDER_SIZE / 2 +
                                                 (rotatedX * 17) / (FixedOne * 16));
    const int16_t screenY = CODEX_RENDER_SIZE / 2 + modelY;
    const int16_t depth = static_cast<int16_t>(rotatedZ / 64);

    for (int8_t oy = -static_cast<int8_t>(splat); oy <= static_cast<int8_t>(splat); ++oy) {
        for (int8_t ox = -static_cast<int8_t>(splat); ox <= static_cast<int8_t>(splat); ++ox) {
            const int16_t x = screenX + ox;
            const int16_t y = screenY + oy;
            if (x < 0 || x >= CODEX_RENDER_SIZE || y < 0 || y >= CODEX_RENDER_SIZE) continue;
            const uint32_t index = static_cast<uint32_t>(y) * CODEX_RENDER_SIZE + x;
            if (depth >= depthBuffer[index]) {
                depthBuffer[index] = depth;
                framebuffer[index] = color;
            }
        }
    }
}

uint16_t faceColor(int8_t modelZ, float sine, float cosine) {
    const float normalX = modelZ > 0 ? sine : -sine;
    const float normalZ = modelZ > 0 ? cosine : -cosine;
    const float diffuse = max(0.0f, normalX * -0.35f + normalZ * 0.88f);
    const float brightness = constrain(0.58f + diffuse * 0.42f, 0.48f, 1.0f);
    return rgb565(static_cast<uint8_t>(245.0f * brightness),
                  static_cast<uint8_t>(245.0f * brightness),
                  static_cast<uint8_t>(240.0f * brightness));
}

}  // namespace

void renderCodexFrame(uint16_t* framebuffer, float rotationRadians) {
    std::fill(framebuffer, framebuffer + CODEX_RENDER_SIZE * CODEX_RENDER_SIZE,
              rgb565(8, 8, 8));
    std::fill(depthBuffer, depthBuffer + CODEX_RENDER_SIZE * CODEX_RENDER_SIZE,
              std::numeric_limits<int16_t>::min());

    const float sineFloat = sinf(rotationRadians);
    const float cosineFloat = cosf(rotationRadians);
    const int16_t sine = static_cast<int16_t>(sineFloat * FixedOne);
    const int16_t cosine = static_cast<int16_t>(cosineFloat * FixedOne);
    const uint16_t front = faceColor(HalfThickness, sineFloat, cosineFloat);
    const uint16_t back = faceColor(-HalfThickness, sineFloat, cosineFloat);

    // Front and rear surfaces. Two-pixel splats close projection sampling gaps.
    for (int16_t py = 0; py < ModelSize; ++py) {
        for (int16_t px = 0; px < ModelSize; ++px) {
            if (!maskAt(px, py)) continue;
            const int16_t x = px - HalfModel;
            const int16_t y = py - HalfModel;
            plotProjected(framebuffer, x, y, -HalfThickness, sine, cosine, back, 1);
            plotProjected(framebuffer, x, y, HalfThickness, sine, cosine, front, 1);
        }
    }

    // Extruded boundary creates visible thickness at oblique and edge-on angles.
    for (int16_t py = 0; py < ModelSize; ++py) {
        for (int16_t px = 0; px < ModelSize; ++px) {
            if (!maskAt(px, py)) continue;
            const bool boundary = !maskAt(px - 1, py) || !maskAt(px + 1, py) ||
                                  !maskAt(px, py - 1) || !maskAt(px, py + 1);
            if (!boundary) continue;
            const int16_t x = px - HalfModel;
            const int16_t y = py - HalfModel;
            for (int8_t z = -HalfThickness; z <= HalfThickness; z += 2) {
                const float highlight = static_cast<float>(z + HalfThickness) /
                                        (HalfThickness * 2);
                const uint8_t sideLevel = static_cast<uint8_t>(62 + 48 * highlight);
                const uint16_t side = rgb565(sideLevel, sideLevel, sideLevel);
                plotProjected(framebuffer, x, y, z, sine, cosine, side, 1);
            }
        }
    }
}
