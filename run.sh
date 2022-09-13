#!/bin/bash
rm core*
make clean && make test && ./mclang test.c >/dev/null
./mclang std/loop.c > /dev/null
./mclang std/if.c > /dev/null
