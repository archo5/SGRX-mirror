@ECHO OFF
mkdir build_w32vs
cd build_w32vs
cmake -G "Visual Studio 14 2015" ..
cd ..
