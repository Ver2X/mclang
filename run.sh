#!/bin/bash
rm core*
cmake -S . -B build
cd build
cmake --build . -j16
cd ..
make clean && make test #&& ./build/mclang test.c >/dev/null
./build/mclang std/loop.c > /dev/null
./build/mclang std/if.c > /dev/null
