#include "game_of_life.hpp"

#include <cassert>
#include <initializer_list>
#include <iostream>
#include <random>
#include <vector>

void test_cell_access() {
    GameOfLife game(5, 5, 2, 1);
    assert(!game.is_alive(2, 1));

    game.set_cell(2, 1, true);
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            assert(game.is_alive(x, y) == (x == 2 && y == 1));
        }
    }

    game.set_cell(2, 1, false);
    assert(!game.is_alive(2, 1));
}

void test_empty_grid() {
    GameOfLife game(5, 5, 3, 2);
    game.run();

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            assert(!game.is_alive(x, y));
        }
    }
}

void test_single_live_cell() {
    GameOfLife game(5, 5, 2, 1);
    game.set_cell(2, 2, true);
    game.run();

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            assert(!game.is_alive(x, y));
        }
    }
}

void test_stable_block() {
    GameOfLife game(6, 5, 4, 2);
    game.set_cell(2, 1, true);
    game.set_cell(3, 1, true);
    game.set_cell(2, 2, true);
    game.set_cell(3, 2, true);
    game.run();

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 6; x++) {
            bool expected = (x == 2 || x == 3) && (y == 1 || y == 2);
            assert(game.is_alive(x, y) == expected);
        }
    }
}

void test_blinker() {
    for (int iterations : {1, 2}) {
        GameOfLife game(5, 5, 3, iterations);
        game.set_cell(1, 2, true);
        game.set_cell(2, 2, true);
        game.set_cell(3, 2, true);
        game.run();

        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                bool expected;
                if (iterations == 1) {
                    expected = x == 2 && y >= 1 && y <= 3;
                } else {
                    expected = y == 2 && x >= 1 && x <= 3;
                }
                assert(game.is_alive(x, y) == expected);
            }
        }
    }
}

void test_wrapped_edges() {
    GameOfLife game(5, 5, 4, 1);
    game.set_cell(4, 2, true);
    game.set_cell(0, 2, true);
    game.set_cell(1, 2, true);
    game.run();

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            bool expected = x == 0 && y >= 1 && y <= 3;
            assert(game.is_alive(x, y) == expected);
        }
    }
}

void test_parallel_generations() {
    std::vector<int> widths = {7, 15, 22, 100, 500};
    std::vector<int> heights = {12, 1, 33, 100, 200};
    std::vector<int> iterations_number = {10, 2, 50, 15, 88};
    for (std::size_t i = 0; i < widths.size(); i++) {
        auto width = widths[i];
        auto height = heights[i];
        auto iterations = iterations_number[i];
        for (int threads : {4, 8, 12}) {
            GameOfLife sequential(width, height, 1, iterations);
            GameOfLife parallel(width, height, threads, iterations);
            std::mt19937 rng(12345);
            std::uniform_int_distribution<int> bit(0, 1);

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    bool alive = bit(rng) == 1;
                    sequential.set_cell(x, y, alive);
                    parallel.set_cell(x, y, alive);
                }
            }

            sequential.run();
            parallel.run();

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    assert(sequential.is_alive(x, y) ==
                           parallel.is_alive(x, y));
                }
            }
        }
    }
}

int main() {
    test_cell_access();
    test_empty_grid();
    test_single_live_cell();
    test_stable_block();
    test_blinker();
    test_wrapped_edges();
    test_parallel_generations();
    std::cerr << "All tests passed!\n";
    return 0;
}
