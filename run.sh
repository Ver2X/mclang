#!/bin/bash
make clean && make test && ./chibicc test.c >/dev/null
