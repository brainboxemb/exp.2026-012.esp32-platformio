#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_heap_caps.h>

#include "hub75_geometry.h"

// Hardware qualification for the EXISTING five-panel assembly:
// 5 electrical 64x32 panels, mounted in portrait -> 160x64 logical pixels.
// No Adafruit GFX, LVGL, WiFi, or application framebuffer in this test.
namespace {

constexpr uint8_t kBrightness = 24; // Conservative initial power-on level.
constexpr uint32_t kPatternIntervalMs = 2000;

MatrixPanel_I2S_DMA *panel = nullptr;

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr Color kBlack = {0, 0, 0};
constexpr Color kWhite = {255, 255, 255};
constexpr Color kRed = {255, 0, 0};
constexpr Color kGreen = {0, 255, 0};
constexpr Color kBlue = {0, 0, 255};
constexpr Color kYellow = {255, 255, 0};
constexpr Color kCyan = {0, 255, 255};
constexpr Color kOrange = {255, 100, 0};

constexpr Color kPanelColors[hub75_geometry::kPanelCount] = {
    kRed, kGreen, kBlue, kCyan, kOrange
};

void pixel(uint16_t x, uint16_t y, const Color &color) {
    hub75_geometry::Point physical{};
    if (hub75_geometry::logicalToDma(x, y, physical)) {
        panel->drawPixelRGB888(physical.x, physical.y,
                              color.r, color.g, color.b);
    }
}

void fillLogical(const Color &color) {
    // Fast physical fill is safe because every physical pixel belongs to
    // exactly one logical pixel; see tests/test_hub75_geometry.cpp.
    panel->fillScreenRGB888(color.r, color.g, color.b);
}

void drawPanelBands() {
    // Five distinct 32x64 stripes: one per portrait-mounted panel.
    for (uint16_t x = 0; x < hub75_geometry::kLogicalWidth; ++x) {
        const Color &color = kPanelColors[x / hub75_geometry::kPanelHeight];
        for (uint16_t y = 0; y < hub75_geometry::kLogicalHeight; ++y) {
            pixel(x, y, color);
        }
    }

    // White asymmetric corners make the viewing direction easy to check.
    for (uint16_t i = 0; i < hub75_geometry::kPanelCount; ++i) {
        const uint16_t left = i * hub75_geometry::kPanelHeight;
        for (uint16_t y = 1; y < 5; ++y) {
            for (uint16_t x = left + 1; x < left + 5; ++x) {
                pixel(x, y, kWhite);
            }
        }
    }
}

void drawOrientation() {
    fillLogical(kBlack);
    for (uint16_t x = 0; x < hub75_geometry::kLogicalWidth; ++x) {
        pixel(x, 0, kGreen); // Top
        pixel(x, hub75_geometry::kLogicalHeight - 1, kBlue); // Bottom
    }
    for (uint16_t y = 0; y < hub75_geometry::kLogicalHeight; ++y) {
        pixel(0, y, kRed); // Left
        pixel(hub75_geometry::kLogicalWidth - 1, y, kYellow); // Right
        const uint16_t diagonalX =
            (uint32_t(y) * (hub75_geometry::kLogicalWidth - 1)) /
            (hub75_geometry::kLogicalHeight - 1);
        pixel(diagonalX, y, kWhite);
    }
}

void showPattern(uint8_t pattern) {
    switch (pattern) {
        case 0: drawPanelBands();  Serial.println("HUB75: five bands (R/G/B/C/O)"); break;
        case 1: drawOrientation(); Serial.println("HUB75: orientation / corners"); break;
        case 2: fillLogical(kRed);   Serial.println("HUB75: RED all five"); break;
        case 3: fillLogical(kGreen); Serial.println("HUB75: GREEN all five"); break;
        case 4: fillLogical(kBlue);  Serial.println("HUB75: BLUE all five"); break;
        default: fillLogical(kBlack); Serial.println("HUB75: BLACK all five"); break;
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("ESP32 HUB75 FIVE-panel test: 160x64 logical / 320x32 DMA");
    Serial.printf("Free DMA-capable heap before: %u bytes\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));

    HUB75_I2S_CFG cfg(hub75_geometry::kPanelWidth,
                      hub75_geometry::kPanelHeight,
                      hub75_geometry::kPanelCount);

    // Existing ESP32 WROOM + HUB75 wiring from Experiment 009.
    cfg.gpio.r1 = 18;
    cfg.gpio.g1 = 26;
    cfg.gpio.b1 = 27;
    cfg.gpio.r2 = 14;
    cfg.gpio.g2 = 12;
    cfg.gpio.b2 = 13;
    cfg.gpio.a = 23;
    cfg.gpio.b = 19;
    cfg.gpio.c = 5;
    cfg.gpio.d = 17;
    cfg.gpio.e = -1; // 64x32 panel: E unused; tied low on existing wiring.
    cfg.gpio.lat = 4;
    cfg.gpio.oe = 15;
    cfg.gpio.clk = 16;

    cfg.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    cfg.clkphase = false;
    cfg.double_buff = false;

    static MatrixPanel_I2S_DMA display(cfg);
    panel = &display;
    if (!panel->begin()) {
        Serial.println("ERROR: HUB75 DMA initialization failed (check DMA RAM)");
        panel = nullptr;
        return;
    }
    Serial.printf("Free DMA-capable heap after: %u bytes\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));
    panel->setBrightness8(kBrightness);
    showPattern(0);
}

void loop() {
    if (panel == nullptr) {
        delay(1000);
        return;
    }

    static uint8_t pattern = 0;
    static uint32_t lastChange = millis();
    const uint32_t now = millis();
    if (now - lastChange >= kPatternIntervalMs) {
        pattern = (pattern + 1) % 6;
        showPattern(pattern);
        lastChange = now;
    }
    delay(10);
}
