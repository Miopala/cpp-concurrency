# SPSC Ring Buffer

C++ implementation of a bounded single-producer, single-consumer queue.

## Build

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/spsc_ring_buffer_test
~~~

## Sanitizers

### ASan + UBSan

~~~sh
cmake -S . -B build-asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON
cmake --build build-asan
./build-asan/spsc_ring_buffer_test
~~~

### TSan

~~~sh
cmake -S . -B build-tsan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TSAN=ON
cmake --build build-tsan
setarch "$(uname -m)" -R ./build-tsan/spsc_ring_buffer_test
~~~
