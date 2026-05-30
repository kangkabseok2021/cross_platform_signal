#pragma once
#include <atomic>
#include <array>
#include <cstddef>

// Single-producer / single-consumer lock-free ring buffer.
// N must be a power of 2; capacity is N-1 samples (one slot is the sentinel).
// head_ is written only by the producer; tail_ only by the consumer.
// The acquire/release pair ensures sample visibility without a mutex —
// a mutex can block the producer indefinitely if the OS preempts the holder
// during an audio callback, causing an audible glitch.
template<typename T, size_t N>
class AudioRingBuffer {
    static_assert(N >= 2 && (N & (N - 1)) == 0, "N must be a power of 2 >= 2");

public:
    // Push one sample. Returns false (non-blocking) when buffer is full.
    [[nodiscard]] bool push(T val) noexcept {
        const size_t h    = head_.load(std::memory_order_relaxed);
        const size_t next = (h + 1) & kMask;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }
        buf_[h] = val;
        head_.store(next, std::memory_order_release);
        return true;
    }

    // Pop one sample into out. Returns false when buffer is empty.
    [[nodiscard]] bool pop(T& out) noexcept {
        const size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }
        out = buf_[t];
        tail_.store((t + 1) & kMask, std::memory_order_release);
        return true;
    }

    // Number of samples currently available for reading.
    [[nodiscard]] size_t available() const noexcept {
        const size_t h = head_.load(std::memory_order_acquire);
        const size_t t = tail_.load(std::memory_order_acquire);
        return (h - t) & kMask;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

private:
    static constexpr size_t kMask = N - 1;

    // Separate cache lines prevent false sharing on Intel/ARM.
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
    std::array<T, N> buf_{};
};
