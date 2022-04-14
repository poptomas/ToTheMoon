# ToTheMoon (Cryptocurrency Trading Bot)

## Table of contents
- [ToTheMoon (Cryptocurrency Trading Bot)](#tothemoon-cryptocurrency-trading-bot)
  - [Table of contents](#table-of-contents)
  - [Introduction](#introduction)
    - [Brief Description](#brief-description)
    - [Motivation](#motivation)
  - [Technology used](#technology-used)
  - [Setup and launch](#setup-and-launch)
  - [Hardware requirements](#hardware-requirements)

## Introduction
### Brief Description

ToTheMoon (TTM) is a console application written in C++ which serves as a simulator for algorithmic trading with cryptocurrencies.
Currently supported cryptocurrency exchange is [Binance](https://www.binance.com/). Within the application there is no real money involved.

TTM supports various commands which are requested from the user via std::cin:
```
-------------------------------------
Supported commands (case insensitive):
withdraw
current
market
history
help
indicators
add [symbol]
remove [symbol]
deposit [value]
-------------------------------------
```
- which is by the way expected output of help command

### Motivation
The main motivation behind the simulator creation was to learn more about cryptocurrency in general
and whether such a volatile field can be somehow predicted. There are plenty of options which can
be considered, for instance, using deep neural networks ([RNN](https://stanford.edu/~shervine/teaching/cs-230/cheatsheet-recurrent-neural-networks) - Recurrent Neural Networks or [LSTM](https://stanford.edu/~shervine/teaching/cs-230/cheatsheet-recurrent-neural-networks#architecture) - Long Short Term Memory) or using time series analysis techniques such as ARIMA.
Nevertheless, an option which was chosen for this task was strictly algorithmic trading using technical indicators which
are very common in the field of "human cryptocurrency trading" and compared to the first two options are easier to test since
most common cryptocurrency exchanges support a variety of technical indicators which can be embedded to a user interface side by side
with the current exchange rate.

## Technology used
- CMake
    - To build the library it is demanded to have at least version 3.15 which was tested by the author
    - For Windows users: CMake should be available within Visual Studio if you have installed [Desktop development with C++ and Linux Development with C++](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170).


- C++
    - Modern ```C++``` (```C++17/C++20```) is required
    - For Windows users: ```C++20``` is required since the program makes use of [std::format](https://en.cppreference.com/w/cpp/utility/format/format)
        - CMake compiles with ```/std:c++latest```
        - Test run (and the majority of development) was made using Visual Studio 2022 with MSVC 19+
    - For Linux users: at least C++17 is required (```std::format``` is not yet supported by modern ```g++``` nor modern ```clang```)
        - CMake compiles with ```-std=c++20```
        - Test run was made using Ubuntu 20.04 (WSL)
        - Build was made both within Visual Studio 2022 (multiple configurations supported) and solely through the command line using ```linux.sh``` script

- External dependencies
    - [vcpkg](https://vcpkg.io) - multiplatform C++ package manager
    - [cpprestsdk](https://github.com/microsoft/cpprestsdk) - Library which takes care of HTTP Requests and JSON data processing in this library

## Setup and launch
- In the root directory there is an install script according to your operating system (windows.bat, linux.sh) which
takes care of [vcpkg](https://vcpkg.io) installation and which is further used to install [cpprestsdk](https://github.com/microsoft/cpprestsdk)
- For Windows users: it is suggested to use Visual Studio 2019/2022 which has CMake installed ([Desktop development with C++ and Linux Development with C++](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170))
- For Linux users: the script takes care of (Debian based) the installation of build-essential packages
necessary for compiling software. Furthermore, it installs [g++-11](https://gcc.gnu.org/projects/cxx-status.html) and [clang++-12](https://clang.llvm.org/cxx_status.html) to make
sure that the library compiles with a compiler which can support C++20.

## Hardware requirements
- Minimal requirements are not set - in the documentation there is the specification of the machine where the project was launched without any observable limitations which could be set as recommmendend hardware requirements