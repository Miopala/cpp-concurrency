#pragma once

#include <atomic>
#include <utility>
#include <vector>

template <typename T> class BoundedQueue {
  public:
    BoundedQueue(int _capacity) : capacity(_capacity + 1) {
        buffer.resize(capacity);
    }

    bool empty(int cur_head) {
        return cur_head == tail.load(std::memory_order_acquire);
    }

    bool empty() {
        return head.load() == tail.load(std::memory_order_acquire);
    }

    bool full(int cur_tail) {
        return (cur_tail + 1) % capacity == head.load(std::memory_order_acquire);
    }

    bool full() {
        return (tail.load(std::memory_order_acquire) + 1) % capacity ==
               head.load(std::memory_order_acquire);
    }

    bool push(T x) {
        int ptr = tail.load(std::memory_order_relaxed);
        if (full(ptr)) {
            return false;
        }

        buffer[ptr] = std::move(x);
        ptr = (ptr + 1) % capacity;
        tail.store(ptr, std::memory_order_release);
        return true;
    }

    bool pop(T &out) {
        int ptr = head.load(std::memory_order_relaxed);
        if (empty(ptr)) {
            return false;
        }

        out = std::move(buffer[ptr]);
        ptr = (ptr + 1) % capacity;
        head.store(ptr, std::memory_order_release);
        return true;
    }

  private:
    const int capacity;
    std::vector<T> buffer;
    alignas(64) std::atomic<int> head{0};
    alignas(64) std::atomic<int> tail{0};
};
