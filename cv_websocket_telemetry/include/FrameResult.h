#pragma once
#include <cstdint>
#include <vector>

struct BoundingRect {
    int x{}, y{}, width{}, height{};
    [[nodiscard]] int area() const noexcept { return width * height; }
};

struct FrameResult {
    uint64_t frame_id{0};
    uint64_t timestamp_ms{0};
    std::vector<BoundingRect> contours;
    [[nodiscard]] bool empty() const noexcept { return contours.empty(); }
};
