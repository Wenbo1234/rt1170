cmake -DCMAKE_TOOLCHAIN_FILE="../../Source/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=flexspi_nor_sdram_debug  .
mingw32-make -j4
IF "%1" == "" ( pause ) 
