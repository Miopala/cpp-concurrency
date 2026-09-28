#pragma once

#include <algorithm>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

class GameOfLife {
  public:
    struct CompletionStep {
        GameOfLife *game_instance;
        void operator()() noexcept { game_instance->end_of_phase_action(); }
    };

  private:
    int width, height;
    int num_threads;
    int num_iterations;
    bool show_board = false;

    std::vector<uint8_t> grid_a;
    std::vector<uint8_t> grid_b;

    std::vector<uint8_t> *read_grid;
    std::vector<uint8_t> *write_grid;

    std::vector<std::thread> workers;

    std::barrier<CompletionStep> sync_barrier;

    inline int get_index(int x, int y) const {
        int wrapped_x = x;
        if (wrapped_x < 0)
            wrapped_x += width;
        else if (wrapped_x >= width)
            wrapped_x -= width;

        int wrapped_y = y;
        if (wrapped_y < 0)
            wrapped_y += height;
        else if (wrapped_y >= height)
            wrapped_y -= height;

        return wrapped_y * width + wrapped_x;
    }

    void end_of_phase_action() noexcept {
        std::swap(read_grid, write_grid);
        if (show_board) {
            std::cout << "\033[H\033[2J";
            print_grid();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    int count_alive_neighbors(int x, int y) const {
        int alive_neighbours = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0)
                    continue;

                if (((*read_grid)[get_index(x + i, y + j)])) {
                    alive_neighbours++;
                }
            }
        }
        return alive_neighbours;
    }

    void worker_thread(int start_row, int end_row) {
        for (int iter = 0; iter < num_iterations; iter++) {
            for (int y = start_row; y < end_row; y++) {
                for (int x = 0; x < width; x++) {
                    int alive = count_alive_neighbors(x, y);
                    const int cur_id = y * width + x;
                    auto current_state = (*read_grid)[cur_id];

                    if (current_state) {
                        (*write_grid)[cur_id] = (alive >= 2 && alive <= 3);
                    } else {
                        (*write_grid)[cur_id] = (alive == 3);
                    }
                }
            }
            sync_barrier.arrive_and_wait();
        }
    }

  public:
    GameOfLife(int w, int h, int threads, int iterations)
        : width(w), height(h), num_threads(threads), num_iterations(iterations),
          grid_a(w * h), grid_b(w * h), read_grid(&grid_a), write_grid(&grid_b),
          sync_barrier(threads, CompletionStep(this)) {}

    void set_show_board(bool enabled) { show_board = enabled; }

    void set_cell(int x, int y, bool alive) {
        (*read_grid)[y * width + x] = static_cast<uint8_t>(alive);
    }

    bool is_alive(int x, int y) const {
        return (*read_grid)[y * width + x] != 0;
    }

    void randomize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        for (auto &cell : *read_grid) {
            cell = static_cast<uint8_t>(dis(gen));
        }
    }

    void run() {
        int rows_per_thread = height / num_threads;

        int rem = height % num_threads;
        int cur = 0;
        for (int i = 0; i < num_threads; i++) {
            int start_row = cur;
            int end_row = start_row + rows_per_thread + (i < rem ? 1 : 0);
            cur = end_row;
            workers.emplace_back(&GameOfLife::worker_thread, this, start_row,
                                 end_row);
        }

        for (auto &t : workers)
            t.join();

        workers.clear();
    }

    void print_grid() const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << ((*read_grid)[get_index(x, y)] ? "#" : ".");
            }
            std::cout << "\n";
        }
    }
};
