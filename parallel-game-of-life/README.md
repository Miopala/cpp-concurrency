# Parallel Game of Life

C++ implementation of parallel Game of Life with double buffering and barrier synchronization.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/parallel_game_of_life_test
```

## Sanitizers

### ASan + UBSan

```sh
cmake -S . -B build-asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON
cmake --build build-asan
./build-asan/parallel_game_of_life_test
```

### TSan

```sh
cmake -S . -B build-tsan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TSAN=ON
cmake --build build-tsan
setarch "$(uname -m)" -R ./build-tsan/parallel_game_of_life_test
```
