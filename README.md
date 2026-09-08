# Help_Make
Help_Make (hm) is a simple, lightweight build system that makes compiling C++ projects easy.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/)

## Features
- Simple Configuration - Define your build settings once in HelpMake.txt
- Command-Line Overrides - Quick changes without editing the config file
- Multiple Compilers - Support for GCC, Clang, and MSVC
- Flexible Input - Specify source files in config or on the command line
- Clean Output - Color-coded messages for errors and success
- Lightweight

## Quick Start
1. Create a HelpMake.txt file in your project (Comments arent supported yet..):
```txt
Compiler: gcc
Version: std=c++17
Output: program.exe
InputFiles {
    main.cpp
    src/file.cpp
}
Includes {
    include
    include/hp
    backends/glfw
}
Github {
    https://github.com/name/repo-name -> include/folder
}
Flags: -Wall
```
2. Build your project:
```bash
hm -b
```
you can also use `hm --build`

## Usage
### Basic Commands
```bash
# Show help
hm -h
hm --help

# Show version
hm -v
hm --version

# Build using HelpMake.txt
hm -b
hm --build

# Build with custom compiler
hm -b -Clang

# Build with command-line options
hm -b -Gcc -std=c++20 -oapp.exe -main.cpp

# Show license
hm -l
hm --license
```

## Command line option | See hm --help

### Options:
 - -h, --help     Output this help message
 - -v, --version  Output the program version
 - -b, --build    Build HelpMake.txt
 - -l, --license  Output the license information

### Avaible Compilers:
 - -Gcc   - GNU Compiler Collection
 - -Clang - LLVM Compiler
 - -MSVC  - Microsoft Visual C++ Compiler

### Avaible Flags:
 - -I<path>     Include a specified directory
 - -F<flags>    Add specified flags
 - -L<library>  include a specified library
### Avaible Versions:
 - -std=c++98
 - -std=c++11
 - -std=c++14
 - -std=c++17
 - -std=c++20
 - -std=c++23
 - -std=c++26
 ### Other
 - --verbose    Make building more verbose
 - --debug      Enable debug mode

## Installation
```bash
git clone https://github.com/Xavi99/Help_Make.git
cd Help_Make
g++ -std=c++17 src/build.cpp src/parser.cpp -o hm.exe
```
Optional :
```bash
setx PATH "%PATH%;C:\path\to\Help_Make"
```

## Requirements
- C++17
- Windows

## License
This program is licensed under the MIT License. See license file for details.

## Contribution
 Feel free to contribute

## Contact
Discord: xxavi640
  
  