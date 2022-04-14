cmake_minimum_required (VERSION 3.16 FATAL_ERROR) 
# 3.16.3 tested on Linux, 3.22.0 on Windows

set(CMAKE_VERBOSE_MAKEFILE ON)
project ("ToTheMoon")
add_executable (ToTheMoon "src/to_the_moon.cpp")
   
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

if(CMAKE_SYSTEM MATCHES Windows)
    # assume that the user is running the program from a reasonably modern Visual Studio
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /std:c++latest") # C++20 did not suffice for std::format
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows")
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/share/cpprestsdk")

elseif(CMAKE_HOST_SYSTEM MATCHES Linux)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -O2 -std=c++20")
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-linux")
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-linux/share/cpprestsdk")

else()
    message("OS not supported")
endif()

find_package(cpprestsdk REQUIRED)
target_link_libraries(ToTheMoon "cpprestsdk::cpprest")

# in case the compilation is run outside of Visual Studio (which specifies its own output destination)
install(TARGETS ToTheMoon DESTINATION "out/build/x64")