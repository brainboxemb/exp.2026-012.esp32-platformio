#include "hub75_geometry.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdio>

int main() {
    using namespace hub75_geometry;

    static_assert(kLogicalWidth == 160 && kLogicalHeight == 64,
                  "Expected logical 160x64");
    static_assert(kDmaWidth == 320 && kDmaHeight == 32,
                  "Expected DMA 320x32");

    std::array<bool, kDmaWidth * kDmaHeight> visited{};
    for (uint16_t x = 0; x < kLogicalWidth; ++x) {
        for (uint16_t y = 0; y < kLogicalHeight; ++y) {
            Point mapped{};
            assert(logicalToDma(x, y, mapped));
            assert(mapped.x < kDmaWidth);
            assert(mapped.y < kDmaHeight);
            // Each 32x64 logical stripe belongs to one 64x32 physical panel.
            assert(mapped.x / kPanelWidth == x / kPanelHeight);
            const std::size_t index = mapped.y * kDmaWidth + mapped.x;
            assert(!visited[index]);
            visited[index] = true;
        }
    }
    for (const bool used : visited) {
        assert(used); // Complete bijection: all 10,240 physical pixels mapped.
    }

    Point p{};
    assert(logicalToDma(0, 0, p) && p.x == 0 && p.y == 31);
    assert(logicalToDma(0, 63, p) && p.x == 63 && p.y == 31);
    assert(logicalToDma(159, 0, p) && p.x == 256 && p.y == 0);
    assert(logicalToDma(159, 63, p) && p.x == 319 && p.y == 0);
    assert(!logicalToDma(160, 0, p));
    assert(!logicalToDma(0, 64, p));

    std::puts("PASS: five-panel 160x64 mapping is complete and bijective");
}
