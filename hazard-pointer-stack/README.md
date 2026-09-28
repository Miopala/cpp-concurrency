# Hazard Pointer Stack

C++ implementation of lock-free stack with hazard pointer memory reclamation.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/hazard_pointer_stack_test
```

## Sanitizers

### ASan + UBSan

```sh
cmake -S . -B build-asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON
cmake --build build-asan
./build-asan/hazard_pointer_stack_test
```

### TSan

```sh
cmake -S . -B build-tsan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TSAN=ON
cmake --build build-tsan
setarch "$(uname -m)" -R ./build-tsan/hazard_pointer_stack_test
```
