@ECHO OFF
mkdir build_w32ninja
cd build_w32ninja
cmake -D CMAKE_BUILD_TYPE=Debug -G "Ninja" ..
cd ..
