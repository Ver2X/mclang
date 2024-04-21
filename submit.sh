#!/bin/bash
rm -rf build
make clean
./format.sh
git add .
git commit
