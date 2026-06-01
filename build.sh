rm -rf build
mkdir build
cmake -DCMAKE_BUILD_TYPE=Debug -B build -S .
cmake --build build
