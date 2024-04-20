#!/bin/bash

# 默认情况下禁用 ASan
ENABLE_ASAN=OFF

# 检查命令行参数，如果传递了 "--asan" 参数，则启用 ASan
if [[ "$1" == "--asan" ]]; then
  ENABLE_ASAN=ON
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake is not installed"
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "Ninja is not installed"
fi
rm core*
if [ ! -d build ]; then
  rm -rf build # incase reuse old .o
  mkdir build
fi
cd build
cmake -DENABLE_ASAN=${ENABLE_ASAN} -G Ninja ..
cmake --build .
cd ..
make clean 
# && make test #&& ./build/mclang test.c >/dev/null
./build/mclang std/loop.c > /dev/null
./build/mclang std/if.c > /dev/null
