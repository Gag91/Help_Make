# HelpMake

**HelpMake (hm)** a simple, lightweight, **language-agnostic** build system.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue.svg)]()

## Features

- **Language-agnostic**       — C, C++, Zig, Rust, Assembly, anything
- **Any compiler**            — `gcc`, `clang`, `msvc`, `zig`, `nasm`, custom
- **Config blocks**           — Define multiple build configs in one file
- **Incremental builds**      — Only recompiles what changed
- **Header tracking**         — Rebuilds when headers change (`.d` files)
- **Fetch dependencies**      — `Fetch { }` auto-clones repos
- **PreBuild / PostBuild**    — Run commands before/after build
- **`compile_commands.json`** — IDE / clangd support
- **Color-coded logs**        — Raw + clean log files
- **Cross-platform**          — Windows and Linux
- **Self-hosting**            — Builds itself with itself

## Quick Start

**1. Create `HelpMake.txt`:**

```txt
Compiler: g++
Version: std=c++20
Output: hm.exe

InputFiles {
    src/main.cpp
    src/file.cpp
}

Includes {
    include
}

Flags {
    -lstdc++exp
}
Config: opt {
    Flags {
        -O2 # Better optimization
    }
}
```

2. Build your project:
```bash
hm -b
```
you can also use `hm --build`

## Usage
### Basic Commands
```bash
# Show help / version / license
hm -h
hm -v
hm -l

# Build using HelpMake.txt
hm -b

# Build a specific config
hm -b --config debug

# Build all configs
hm -b --config-all

# Force rebuild everything
hm -b --rebuild

# Build with any compiler
hm -b -C=zig
hm -b -C=nasm

# Custom version
hm -b -V=std=c99

# Generate compile_commands.json for IDEs
hm -b --json

# Clean build artifacts
hm --clean

# Inspect config
hm --show
hm --dump

# Use a custom config file
hm -b -f custom.txt

# Build from command line (no config file)
hm -b --nofile -C=gcc main.c -o program.exe

# Generate a HelpMake.txt from command-line arguments
hm -b --create -C=gcc main.c -o program.exe

# Generate a custom config file from command-line arguments
hm -b --create -C=gcc main.c -o program.exe -f custom.txt
```

## Command-line Argument | see --help
### Options:
  -h, --help     Output this help message
  -v, --version  Output the program version
  -b, --build    Build using HelpMake.txt
  -l, --license  Output the license information
  -f, --file     Specify a custom HelpMake file
      --show     Show current configuration
      --dump     Print raw HelpMake.txt contents
      --create   Generate HelpMake.txt from build arguments
      --clean    Delete build artifacts

### Build Options (use with -b):
  -Gcc           Use GNU Compiler Collection
  -Clang         Use LLVM Clang Compiler
  -MSVC          Use Microsoft Visual C++ Compiler, Windows only
  -Zig           Use Zig Compiler (C++ support)
  -Sere          Use Sere Compiler
  -C=<compiler>  Use any compiler (e.g., -C=gcc, -C=python)
  -V=<version>   Use any version (e.g., -V=std=c99)

### Build Flags:
  -I<path>       Include a specified directory
  -L<path>       Link a specified library path
  -F<flags>      Add specified compiler flags
  -o<file>       Set output filename
  -r, --run      Run output file after compilation
      --config   Build a specific config from HelpMake.txt
      --nofile   Build from command line only (no config file)
      --verbose  Show detailed build output
      --debug    Show debug information
      --rebuild  Force full rebuild (ignore timestamps)
      --json     Generate compile_commands.json
      --no-sep   Disable incremental build (one-shot compile)
      --clean    Clean build (delete build/HelpMake directory)

## HelpMake.txt
### Basic fields
```txt
Compiler: g++          # Any compiler
Version:  std=c++23    # Any version
Output:   build/app    # Output path
Flags:    -Wall -O2    # Compiler flags
```
### Blocks
```txt
InputFiles {
    src/*.cpp
}

Includes {
    include
    external/glfw/include
}

Flags {
    -Wall
    -O2
}

Fetch {
    https://github.com/user/repo.git -> include/repo
}

PreBuild {
    echo "Starting build..."
}

PostBuild {
    xcopy /E /I /Y assets build\assets
}
```

### Configs
```txt
Compiler: g++
Version: std=c++20
Output: build/app.exe
InputFiles { 
    src/*.cpp 
}

Config: debug {
    Flags: -g -O0 -DDEBUG
    Output: build/debug.exe
}

Config: release {
    Compiler: clang
    Flags: -O3 -DNDEBUG -flto
    Output: build/release.exe
}
```
Build whit
```bash
hm -b --config debug
hm -b --config release
```

## Installation
```bash
git clone https://github.com/Gag91/Help_Make.git
cd Help_Make
g++ -std=c++20 src/*.cpp -o hm.exe -Iinclude
```

Optional :
```bash
setx PATH "%PATH%;C:\path\to\Help_Make"
```

## Requirements
- C++20

## License
This program is licensed under the MIT License. See license file for details.

## Contribution
 Feel free to contribute

## Contact
Discord: xxavi640
  
  
