git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install cpprestsdk:x64-windows
.\vcpkg\vcpkg integrate install
pause