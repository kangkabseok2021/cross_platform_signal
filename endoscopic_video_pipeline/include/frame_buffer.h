#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>

// C++20 Concept — constrains all buffer types passed to SobelProcessor.
// Any non-conforming type produces a clear concept-failure diagnostic.
template<typename B>
concept FrameBuffer =
    requires(B b) {
        { b.data()   } -> std::convertible_to<uint8_t*>;
        { b.size()   } -> std::same_as<size_t>;
        { b.width()  } -> std::same_as<int>;
        { b.height() } -> std::same_as<int>;
    };

// CPU-backed frame buffer — always available (used for CPU Sobel path).
class HeapFrameBuffer {
public:
    HeapFrameBuffer(int w, int h)
        : w_(w), h_(h),
          buf_(std::make_unique<uint8_t[]>(static_cast<size_t>(w * h))) {}

    HeapFrameBuffer(const HeapFrameBuffer&)            = delete;
    HeapFrameBuffer& operator=(const HeapFrameBuffer&) = delete;

    uint8_t* data()          noexcept { return buf_.get(); }
    size_t   size()  const noexcept { return static_cast<size_t>(w_ * h_); }
    int      width() const noexcept { return w_; }
    int      height()const noexcept { return h_; }

private:
    int w_, h_;
    std::unique_ptr<uint8_t[]> buf_;
};

static_assert(FrameBuffer<HeapFrameBuffer>,
              "HeapFrameBuffer must satisfy FrameBuffer concept");
