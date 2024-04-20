#!/bin/bash
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
cmake -G Ninja ..
cmake --build .
cd ..
make clean 
# && make test #&& ./build/mclang test.c >/dev/null
./build/mclang std/loop.c > /dev/null
./build/mclang std/if.c > /dev/null
