#!/bin/bash
rm -rf build 
mkdir build 
cd build 
cmake ..
make -j$(nproc)
./sandbox/sandbox
cd ..