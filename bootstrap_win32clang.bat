@ECHO OFF
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_CC_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G "MinGW Makefiles" ..
cd ..
