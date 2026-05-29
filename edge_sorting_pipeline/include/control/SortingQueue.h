#ifndef SORTING_QUEUE_H
#define SORTING_QUEUE_H

#include <atomic>
#include <array>
#include <cstddef>

template<typename T, std::size_t N>
class SortingQueue {
private:
    std::array<T, N> buffer_;
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
public:
    bool push(const T& item) {
        std::size_t head = head_.load(std::memory_order_relaxed);
        std::size_t next = (head + 1) % N;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // Queue full
        }
        buffer_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        std::size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }
        item = buffer_[tail];
        tail_.store((tail + 1) % N, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return tail_.load(std::memory_order_relaxed) == head_.load(std::memory_order_relaxed);
    }

    std::size_t size() const {
        std::size_t head = head_.load(std::memory_order_relaxed);
        std::size_t tail = tail_.load(std::memory_order_relaxed);
        if (head >= tail) {
            return head - tail;
        }
        return N - (tail - head);
    }
};

#endif // SORTING_QUEUE_H
