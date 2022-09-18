#!/bin/bash
rm core*
if [ ! -d build ]; then
  rm -rf build # incase reuse old .o
  mkdir build
fi
cd build
cmake -G Ninja ..
cmake --build .
cd ..
make clean && make test #&& ./build/mclang test.c >/dev/null
./build/mclang std/loop.c > /dev/null
./build/mclang std/if.c > /dev/null
