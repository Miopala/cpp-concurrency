#include "spsc_ring_buffer.hpp"

#include <cassert>
#include <iostream>
#include <thread>

void test_fifo_basic_functionality() {
    BoundedQueue<int> q(3);

    assert(q.empty());
    assert(q.push(1));
    assert(q.push(2));
    assert(q.push(3));
    assert(q.full());
    assert(!q.push(4));

    int value;
    assert(q.pop(value) && value == 1);
    assert(q.pop(value) && value == 2);
    assert(q.pop(value) && value == 3);
    assert(q.empty());
    assert(!q.pop(value));
}

void test_wraparound() {
    BoundedQueue<int> q(2);
    int value;

    assert(q.push(1));
    assert(q.push(2));
    assert(q.pop(value) && value == 1);
    assert(q.push(3));
    assert(q.pop(value) && value == 2);
    assert(q.pop(value) && value == 3);
    assert(!q.pop(value));
}

void test_concurrent_fifo() {
    BoundedQueue<int> q(3);
    constexpr int iterations = 10000;

    std::jthread producer([&]() {
        for (int i = 0; i < iterations; ++i) {
            while (!q.push(i)) {
                std::this_thread::yield();
            }
        }
    });

    std::jthread consumer([&]() {
        for (int i = 0; i < iterations; ++i) {
            int value;
            while (!q.pop(value)) {
                std::this_thread::yield();
            }
            assert(value == i);
        }
    });

    producer.join();
    consumer.join();
    assert(q.empty());
}

int main() {
    test_fifo_basic_functionality();
    test_wraparound();
    test_concurrent_fifo();
    std::cerr << "All tests passed!\n";
}
