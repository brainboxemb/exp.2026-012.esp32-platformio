#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Hardware smoke test for ONE electrical 64x32 HUB75 panel.
// This is not the five-panel portrait mapping or the display renderer.
namespace {
constexpr uint16_t kWidth = 64;
constexpr uint16_t kHeight = 32;
constexpr uint8_t kBrightness = 48;

MatrixPanel_I2S_DMA *panel = nullptr;

struct Color {
    const char *name;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr Color kColors[] = {
    {"RED", 255, 0, 0},
    {"GREEN", 0, 255, 0},
    {"BLUE", 0, 0, 255},
    {"WHITE", 255, 255, 255},
    {"BLACK", 0, 0, 0},
};

void showColor(size_t index) {
    const Color &color = kColors[index];
    panel->fillScreenRGB888(color.r, color.g, color.b);
    Serial.printf("HUB75: %s\n", color.name);
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("ESP32 HUB75 single-panel test");

    HUB75_I2S_CFG cfg(kWidth, kHeight, 1);

    // Existing known wiring from Experiment 009 (ESP32 WROOM DevKit).
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
    cfg.gpio.e = -1;  // 64x32 panel: E not driven; tie E low if required.
    cfg.gpio.lat = 4;
    cfg.gpio.oe = 15;
    cfg.gpio.clk = 16;

    cfg.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    cfg.clkphase = false;
    cfg.double_buff = false;

    static MatrixPanel_I2S_DMA display(cfg);
    panel = &display;
    if (!panel->begin()) {
        Serial.println("ERROR: HUB75 DMA initialization failed");
        panel = nullptr;
        return;
    }

    panel->setBrightness8(kBrightness);
    showColor(0);
}

void loop() {
    if (panel == nullptr) {
        delay(1000);
        return;
    }

    static size_t index = 0;
    static uint32_t lastChange = millis();
    const uint32_t now = millis();
    if (now - lastChange >= 1500) {
        index = (index + 1) % (sizeof(kColors) / sizeof(kColors[0]));
        showColor(index);
        lastChange = now;
    }
    delay(10);
}
