#!/bin/sh
cmake -DCMAKE_TOOLCHAIN_FILE="../../Source/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=sdram_release  .
make -j4
