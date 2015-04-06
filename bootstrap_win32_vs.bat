@ECHO OFF
mkdir build_w32vs
cd build_w32vs
cmake -D CMAKE_BUILD_TYPE=Debug -G "Visual Studio 12 2013" ..
cd ..
