rm -rf build
mkdir build

cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    -GNinja -B build -S .

cmake --build build
