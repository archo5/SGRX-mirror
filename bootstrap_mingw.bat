@ECHO OFF
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..
cd ..
