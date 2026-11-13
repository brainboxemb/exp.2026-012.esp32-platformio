#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "hub75_geometry.h"

#if LV_USE_THEME_DEFAULT || LV_USE_THEME_SIMPLE || LV_USE_THEME_MONO
#error "LVGL qualification must not build themes"
#endif
#if !LV_USE_LABEL || LV_USE_BUTTON || LV_USE_GRID || LV_USE_FLEX
#error "LVGL qualification permits only the label widget"
#endif
#if LV_USE_STDLIB_MALLOC != LV_STDLIB_CLIB
#error "LVGL must use the C heap, not the built-in static heap"
#endif

namespace {

constexpr uint8_t kBrightness = 24;  // Keep the validated ~9.4% level.
constexpr uint16_t kWidth = hub75_geometry::kLogicalWidth;
constexpr uint16_t kHeight = hub75_geometry::kLogicalHeight;
constexpr uint16_t kRowsPerBuffer = 8;

// 160 x 8 x 2 bytes = 2560 bytes. No full-screen GUI framebuffer.
uint16_t drawBuffer[kWidth * kRowsPerBuffer] = {};
MatrixPanel_I2S_DMA *panel = nullptr;

void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    // LVGL delivers tightly packed RGB565 rows in a partial draw buffer.
    const auto *rgb565 = reinterpret_cast<const uint16_t *>(pixels);
    const uint16_t areaWidth = uint16_t(area->x2 - area->x1 + 1);
    for (int16_t y = area->y1; y <= area->y2; ++y) {
        for (int16_t x = area->x1; x <= area->x2; ++x) {
            const uint16_t packed = *rgb565++;
            hub75_geometry::Point physical{};
            if (!hub75_geometry::logicalToDma(uint16_t(x), uint16_t(y), physical)) {
                continue;
            }
            const uint8_t r5 = uint8_t((packed >> 11) & 0x1f);
            const uint8_t g6 = uint8_t((packed >> 5) & 0x3f);
            const uint8_t b5 = uint8_t(packed & 0x1f);
            panel->drawPixelRGB888(physical.x, physical.y,
                                  uint8_t((r5 << 3) | (r5 >> 2)),
                                  uint8_t((g6 << 2) | (g6 >> 4)),
                                  uint8_t((b5 << 3) | (b5 >> 2)));
        }
    }
    (void)areaWidth;  // Pixel pointer advances exactly once for each pixel.
    lv_display_flush_ready(display);
}

lv_obj_t *newLabel(lv_obj_t *parent, const char *text, int x, int y,
                   uint32_t color) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

void createGui() {
    lv_display_t *display = lv_display_create(kWidth, kHeight);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, drawBuffer, nullptr, sizeof(drawBuffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);

    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, kWidth, 13);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xB5FC00), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    newLabel(header, "LVGL 9.6", 3, 3, 0x101820);
    newLabel(screen, "5x HUB75 / 160x64", 3, 20, 0xF2F2F2);
    newLabel(screen, "RGB565  10MHz", 3, 34, 0xB5FC00);
    newLabel(screen, "BRIGHTNESS: 10%", 3, 48, 0xF2F2F2);
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("ESP32 LVGL + HUB75 five-panel qualification");
    Serial.printf("Free DMA-capable heap before: %u\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));
    Serial.printf("Free heap before: %u\n", unsigned(ESP.getFreeHeap()));

    HUB75_I2S_CFG cfg(hub75_geometry::kPanelWidth,
                      hub75_geometry::kPanelHeight,
                      hub75_geometry::kPanelCount);

    cfg.gpio.r1 = 18; cfg.gpio.g1 = 26; cfg.gpio.b1 = 27;
    cfg.gpio.r2 = 14; cfg.gpio.g2 = 12; cfg.gpio.b2 = 13;
    cfg.gpio.a = 23; cfg.gpio.b = 19; cfg.gpio.c = 5; cfg.gpio.d = 17;
    cfg.gpio.e = -1; cfg.gpio.lat = 4; cfg.gpio.oe = 15; cfg.gpio.clk = 16;
    cfg.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    cfg.clkphase = false;
    cfg.double_buff = false;

    static MatrixPanel_I2S_DMA dma(cfg);
    panel = &dma;
    if (!panel->begin()) {
        Serial.println("ERROR: HUB75 initialization failed");
        panel = nullptr;
        return;
    }
    panel->setBrightness8(kBrightness);

    lv_init();
    createGui();
    lv_timer_handler();  // Draw the initial frame promptly.
    Serial.printf("Free DMA-capable heap after: %u\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));
    Serial.printf("Free heap after: %u\n", unsigned(ESP.getFreeHeap()));
    Serial.println("LVGL screen rendered");
}

void loop() {
    if (!panel) {
        delay(1000);
        return;
    }
    static uint32_t lastTick = millis();
    const uint32_t now = millis();
    const uint32_t elapsed = now - lastTick;
    if (elapsed) {
        lv_tick_inc(elapsed);
        lastTick = now;
    }
    lv_timer_handler();
    delay(5);
}
