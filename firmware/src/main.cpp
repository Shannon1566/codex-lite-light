#include <Arduino.h>

#include "board_config.h"
#include "codex_renderer.h"
#include "st7789_display.h"

enum class LightState : char {
    Off = 'O',
    Idle = 'I',
    Working = 'W',
    Waiting = 'A',
    Success = 'S',
    Error = 'E',
};

namespace {

LightState currentState = LightState::Off;
uint32_t stateChangedMs = 0;
uint32_t lastCommandMs = 0;
St7789Display display;
uint32_t lastAnimationMs = 0;
uint16_t animationBuffer[CODEX_RENDER_SIZE * CODEX_RENDER_SIZE];
float animationAngle = 0.0f;

constexpr uint32_t AnimationIntervalMs = 33;
constexpr uint16_t AnimationOffset =
    (St7789Display::Width - CODEX_RENDER_SIZE) / 2u;

void ledWrite(uint8_t gpio, bool on) {
    digitalWrite(gpio, CODEX_LIGHT_ACTIVE_HIGH ? on : !on);
}

void allLedsOff() {
    ledWrite(CODEX_LIGHT_GPIO_GREEN, false);
    ledWrite(CODEX_LIGHT_GPIO_YELLOW, false);
    ledWrite(CODEX_LIGHT_GPIO_RED, false);
}

bool isStateCommand(char command) {
    switch (command) {
        case static_cast<char>(LightState::Off):
        case static_cast<char>(LightState::Idle):
        case static_cast<char>(LightState::Working):
        case static_cast<char>(LightState::Waiting):
        case static_cast<char>(LightState::Success):
        case static_cast<char>(LightState::Error):
            return true;
        default:
            return false;
    }
}

uint16_t stateColor(LightState state) {
    switch (state) {
        case LightState::Idle:
        case LightState::Success:
            return St7789Display::rgb565(0, 180, 45);
        case LightState::Working:
            return St7789Display::rgb565(255, 180, 0);
        case LightState::Waiting:
            return St7789Display::rgb565(255, 90, 0);
        case LightState::Error:
            return St7789Display::rgb565(220, 0, 0);
        case LightState::Off:
        default:
            return St7789Display::rgb565(0, 0, 0);
    }
}

void setState(LightState state, uint32_t nowMs) {
    currentState = state;
    stateChangedMs = nowMs;
    lastCommandMs = nowMs;
    if (state == LightState::Working) {
        display.fillScreen(St7789Display::rgb565(0, 0, 0));
        animationAngle = 0.0f;
        lastAnimationMs = nowMs;
        renderCodexFrame(animationBuffer, animationAngle);
        display.drawRgb565(AnimationOffset, AnimationOffset, CODEX_RENDER_SIZE,
                           CODEX_RENDER_SIZE, animationBuffer);
    } else {
        display.fillScreen(stateColor(state));
    }
}

void updateDisplayAnimation(uint32_t nowMs) {
    if (currentState != LightState::Working ||
        nowMs - lastAnimationMs < AnimationIntervalMs) {
        return;
    }
    lastAnimationMs = nowMs;
    animationAngle += 0.075f;
    if (animationAngle >= 6.2831853f) animationAngle -= 6.2831853f;
    renderCodexFrame(animationBuffer, animationAngle);
    display.drawRgb565(AnimationOffset, AnimationOffset, CODEX_RENDER_SIZE,
                       CODEX_RENDER_SIZE, animationBuffer);
}

void updateLeds(uint32_t nowMs) {
    const bool blinkOn = ((nowMs / CODEX_LIGHT_BLINK_INTERVAL_MS) % 2u) == 0u;
    allLedsOff();
    switch (currentState) {
        case LightState::Idle:
        case LightState::Success:
            ledWrite(CODEX_LIGHT_GPIO_GREEN, true);
            break;
        case LightState::Working:
            ledWrite(CODEX_LIGHT_GPIO_YELLOW, blinkOn);
            break;
        case LightState::Waiting:
            ledWrite(CODEX_LIGHT_GPIO_YELLOW, true);
            ledWrite(CODEX_LIGHT_GPIO_RED, blinkOn);
            break;
        case LightState::Error:
            ledWrite(CODEX_LIGHT_GPIO_RED, true);
            break;
        case LightState::Off:
        default:
            break;
    }
}

void handleCommand(char command, uint32_t nowMs) {
    if (command == '?') {
        Serial.print("STATE ");
        Serial.println(static_cast<char>(currentState));
        return;
    }
    if (!isStateCommand(command)) {
        Serial.println("ERR UNKNOWN");
        return;
    }
    setState(static_cast<LightState>(command), nowMs);
    Serial.print("OK ");
    Serial.println(command);
}

}  // namespace

void setup() {
    pinMode(CODEX_LIGHT_GPIO_GREEN, OUTPUT);
    pinMode(CODEX_LIGHT_GPIO_YELLOW, OUTPUT);
    pinMode(CODEX_LIGHT_GPIO_RED, OUTPUT);
    allLedsOff();

    display.begin();
    display.showSelfTest();

    Serial.begin(115200);
    stateChangedMs = millis();
    lastCommandMs = stateChangedMs;
}

void loop() {
    const uint32_t nowMs = millis();
    while (Serial.available() > 0) {
        const char input = static_cast<char>(Serial.read());
        if (input != '\r' && input != '\n') handleCommand(input, nowMs);
    }

    if (currentState == LightState::Success &&
        nowMs - stateChangedMs >= CODEX_LIGHT_SUCCESS_HOLD_MS) {
        setState(LightState::Idle, nowMs);
    }
    if (currentState != LightState::Off &&
        nowMs - lastCommandMs >= CODEX_LIGHT_LINK_TIMEOUT_MS) {
        setState(LightState::Off, nowMs);
    }

    updateLeds(nowMs);
    updateDisplayAnimation(nowMs);
    delay(10);
}
