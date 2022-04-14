#!/bin/bash
sudo apt -y install build-essential
sudo apt -y install g++-11
sudo apt -y install clang
sudo apt -y install clang-12
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
vcpkg/vcpkg install cpprestsdk:x64-linux
cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="out/build/linux" .
#For Clang users:
#cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++-12 -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="out/build/linux" .
make
clear
./out/build/linux/ToTheMoon