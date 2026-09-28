#include "hazard_pointer_stack.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

void test_empty_stack() {
    lock_free_stack<int> stack;
    assert(stack.pop() == nullptr);
}

void test_minimalistic_push_pop() {
    lock_free_stack<std::string> stack;
    stack.push("Example1");
    stack.push("Example2");
    assert(*stack.pop() == "Example2");
    assert(*stack.pop() == "Example1");
    assert(stack.pop() == nullptr);
}

static constexpr int OPERATIONS = 10000;
static constexpr int PRODUCER_THREADS = 8;
static constexpr int CONSUMER_THREADS = 8;

void test_concurrent_push_pop() {
    lock_free_stack<int> stack;
    std::vector<std::jthread> threads;
    for (int i = 1; i <= PRODUCER_THREADS; i++) {
        threads.emplace_back(([&stack]() {
            for (int j = 1; j <= OPERATIONS; j++) {
                stack.push(j);
            }
        }));
    }

    for (int i = 1; i <= CONSUMER_THREADS; i++) {
        threads.emplace_back(([&stack]() {
            for (int j = 1; j <= OPERATIONS; j++) {
                stack.pop();
            }
        }));
    }
}

void test_concurrent_push_pop_values() {
    lock_free_stack<int> stack;
    std::vector<std::jthread> threads;
    for (int i = 1; i <= PRODUCER_THREADS; i++) {
        threads.emplace_back(([i, &stack]() {
            for (int j = 1; j <= OPERATIONS; j++) {
                stack.push(i * OPERATIONS + j);
            }
        }));
    }

    threads.clear();
    std::set<int> popped_ids;
    std::mutex mtx;
    for (int i = 1; i <= CONSUMER_THREADS; i++) {

        threads.emplace_back([&mtx, &popped_ids, &stack]() {
            for (int j = 1; j <= OPERATIONS; j++) {
                std::shared_ptr<int> stack_top = stack.pop();
                if (stack_top != nullptr) {
                    std::lock_guard<std::mutex> lck(mtx);
                    popped_ids.insert(*stack_top);
                }
            }
        });
    }

    threads.clear();
    assert(popped_ids.size() == OPERATIONS * PRODUCER_THREADS);
}

int main() {
    test_empty_stack();
    test_minimalistic_push_pop();
    test_concurrent_push_pop();
    test_concurrent_push_pop_values();
    std::cerr << "All tests passed!\n";
    return 0;
}