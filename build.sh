# rm -rf build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -B build -S .
cmake --build build
