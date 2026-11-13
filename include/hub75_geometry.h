#pragma once

#include <stdint.h>

// Five electrical 64x32 panels form a 64x160 chain when laid out vertically.
// In the enclosure they are turned 90 degrees clockwise: logical 160x64.
// Mirrors CHAIN_TOP_RIGHT_DOWN + rotation 1 from VirtualMatrixPanel without
// requiring Adafruit GFX (NO_GFX is enabled for this qualification target).
namespace hub75_geometry {

constexpr uint16_t kPanelWidth = 64;
constexpr uint16_t kPanelHeight = 32;
constexpr uint16_t kPanelCount = 5;
constexpr uint16_t kLogicalWidth = kPanelCount * kPanelHeight;  // 160
constexpr uint16_t kLogicalHeight = kPanelWidth;               // 64
constexpr uint16_t kDmaWidth = kPanelCount * kPanelWidth;       // 320
constexpr uint16_t kDmaHeight = kPanelHeight;                  // 32

struct Point {
    uint16_t x;
    uint16_t y;
};

inline bool logicalToDma(uint16_t x, uint16_t y, Point &out) {
    if (x >= kLogicalWidth || y >= kLogicalHeight) {
        return false;
    }

    // Undo the 90-degree clockwise logical orientation. The unrotated
    // virtual layout has width 64 and height 5 * 32 = 160.
    const uint16_t virtualX = y;
    const uint16_t virtualY = kLogicalWidth - 1 - x;

    const uint16_t row = virtualY / kPanelHeight;
    const uint16_t rowY = virtualY % kPanelHeight;

    // CHAIN_TOP_RIGHT_DOWN is serpentine: every second panel is inverted.
    if ((row & 1U) != 0U) {
        out.x = (kPanelCount - row) * kPanelWidth - 1 - virtualX;
        out.y = kPanelHeight - 1 - rowY;
    } else {
        out.x = (kPanelCount - row - 1) * kPanelWidth + virtualX;
        out.y = rowY;
    }
    return true;
}

}  // namespace hub75_geometry
