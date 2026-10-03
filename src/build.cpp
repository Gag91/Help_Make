#include "hp/borderStyle/border.hpp"
#include "hp/colors/color.hpp"
#include "hp/other/other.hpp"
#include "parser.hpp"
#include <filesystem>
#include <fstream>

class Build {
  private:
    hp::BorderChars chars = hp::getBorderChars(hp::EXTENDED);

  public:
    Build() = default;
    void help() const {
        constexpr std::string_view help = R"(Help_Make (hm) - Build System
Use : hm [options]

Options:
  -h, --help     Output this help message
  -v, --version  Output the program version
  -b, --build    Build using HelpMake.txt
  -l, --license  Output the license information
  -f, --file     Specify a custom HelpMake file
      --show     Show current configuration
      --dump     Print raw HelpMake.txt contents
      --create   Generate HelpMake.txt from build arguments

Build Options (use with -b):
  -Gcc           Use GNU Compiler Collection
  -Clang         Use LLVM Clang Compiler
  -MSVC          Use Microsoft Visual C++ Compiler, Windows only
  -Zig           Use Zig Compiler (C/C++ support)
  -Sere          Use Sere Compiler

Build Flags:
  -I<path>       Include a specified directory
  -L<path>       Link a specified library path
  -F<flags>      Add specified compiler flags
  -o<file>       Set output filename
  -r, --run      Run output file after compilation
      --nofile   Build from command line only (no config file)
      --verbose  Show detailed build output
      --debug    Show debug information

Available Versions:
  -std=c++98
  -std=c++11
  -std=c++14
  -std=c++17
  -std=c++20
  -std=c++23
  -std=c++26

Examples:
  Build from HelpMake.txt
    hm -b

  Build with a specific compiler
    hm -b -clang -o main.exe

  Build from command line (no config file)
    hm -b --nofile -gcc main.cpp -o main.exe

  Generate a HelpMake.txt from arguments
    hm -b -Gcc -std=c++26 -F-Wall -Iinclude src/main.cpp -o main.exe --create

  Use a custom config file
    hm -b -f custom.txt

  Show current config / raw file
    hm --show
    hm --dump
)";

        std::cout << help << "\n";
    }

    void version() const {
        std::cout << "Help_Make (hm) - Build System\n";
        std::cout << "Version: 5.0.0\n";
        std::cout << "Author : Xavi99\n";
        std::cout << "Website: https://github.com/Xavi99/Help_Make\n";
        std::cout << "This program is licensed under the MIT License. See \"--license\" for details.\n\n";
    }
    void license(hp::BorderStyle style) const {
        hp::BorderChars chars = hp::getBorderChars(style);
        std::cout << "Help_Make (hm) - License Information\n";
        constexpr std::string_view license = R"(MIT License

Copyright (c) 2026 Xavi99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)";
        std::cout << license << "\n";
    }
};

int main(int argc, char *argv[]) {
    hp::enableUTF8();
    std::string compiler = "";
    std::string version = "";
    std::string output = "";
    std::string flags = "";
    std::string inputFile = "";
    std::string filename = "HelpMake.txt";

    bool verbose = false;
    bool run = false;
    bool debug = false;
    bool n_file = false;
    bool sere = false;
    bool create = false;
    bool allCfgs = false;
    bool json = false;
    bool seperate = true;
    bool rebuild = false;

    std::vector<std::string> selectedConfigs;
    Build build;

    if (argc < 2) {
        build.help();
        return 0;
    }

    std::string_view argument = argv[1];

    if (argument == "--help" || argument == "-h") {
        build.help();
        return 0;
    } else if (argument == "--version" || argument == "-v") {
        build.version();
        return 0;
    } else if (argument == "--license" || argument == "-l") {
        build.license(hp::EXTENDED);
        return 0;
    } else if (argument == "--dump") {
        if (std::filesystem::exists(filename)) {
            std::ifstream file(filename);
            std::cout << file.rdbuf() << "\n";
        }
    } else if (argument == "--clean") {
        std::filesystem::remove_all("build/HelpMake");
        hp::printlnCl("[HelpMake] Cleaned build/HelpMake directory.", hp::Color::GREEN);
    } else if (argument == "--show") {
        Parser parser(filename, inputFile, compiler, version, output, flags, verbose, run, debug, n_file, sere, create, allCfgs, json, seperate, rebuild);
        parser.parse();

        std::cout << "Compiler: " << parser.getCompiler() << "\n";
        std::cout << "Version:  " << parser.getVersion() << "\n";
        std::cout << "Output:   " << parser.getOutput() << "\n\n";

        auto files = parser.getInputFile();
        if (!files.empty()) {
            std::cout << "InputFiles:\n";
            for (const auto &f : files) {
                std::cout << "    " << f << "\n";
            }
            std::cout << "\n";
        }

        auto includes = parser.getInclude();
        if (!includes.empty()) {
            std::cout << "IncludeFiles:\n";
            for (const auto &i : includes) {
                std::cout << "    " << i << "\n";
            }
            std::cout << "\n";
        }

        auto flags_ = parser.getFlags();
        if (!flags_.empty()) {
            std::cout << "Flags:\n";
            for (const auto &f : flags_) {
                std::cout << "    " << f << "\n";
            }
            std::cout << "\n";
        }

        auto modules_ = parser.getModules();
        if (!modules_.empty()) {
            std::cout << "Modules:\n";
            for (const auto &m : modules_) {
                std::cout << "    " << m << "\n";
            }
            std::cout << "\n";
        }

        auto github_ = parser.getGithub();
        if (!github_.empty()) {
            std::cout << "Github:\n";
            for (const auto &g : github_) {
                std::cout << "    " << g << "\n";
            }
            std::cout << "\n";
        }

        return 0;
    } else if (argument == "--build" || argument == "-b") {
        for (int i = 2; i < argc; i++) {
            std::string_view arg = argv[i];

            if (arg == "-Gcc" || arg == "-gcc") {
                compiler = "gcc";
            } else if (arg == "-Clang" || arg == "-clang") {
                compiler = "clang";
            } else if (arg == "-MSVC" || arg == "-msvc") {
                compiler = "msvc";
            } else if (arg == "-Zig" || arg == "-zig") {
                compiler = "zig";
            } else if (arg == "-sere" || arg == "-Sere") {
                compiler = "sere";
                sere = true;

            } else if (arg == "-std=c++98" || arg == "-std=c++11" || arg == "-std=c++14" ||
                       arg == "-std=c++17" || arg == "-std=c++20" || arg == "-std=c++23" ||
                       arg == "-std=c++26") {
                version = std::string(arg).substr(1);

            } else if (arg == "-o" || arg == "--output") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    output = argv[++i];
                } else {
                    hp::printlnCl("Error: Output file not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }

            } else if (arg.rfind("-o", 0) == 0 && arg.size() > 2) {
                output = std::string(arg).substr(2);

            } else if (arg == "-F" || arg == "--Flag") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    if (!flags.empty())
                        flags += " ";
                    flags += argv[++i];
                } else {
                    hp::printlnCl("Error: Additional flags not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }

            } else if (arg.rfind("-F", 0) == 0 && arg.size() > 2) {
                std::string n_flags = std::string(arg).substr(2);
                if (!flags.empty())
                    flags += " ";
                flags += n_flags;

            } else if (arg == "-I" || arg == "--Include") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    if (!flags.empty())
                        flags += " ";
                    flags += "-I" + std::string(argv[++i]);
                } else {
                    hp::printlnCl("Error: Include path not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }

            } else if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
                std::string includePath = std::string(arg).substr(2);
                if (!flags.empty())
                    flags += " ";
                flags += "-I" + includePath;

            } else if (arg == "-L" || arg == "--Library") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    if (!flags.empty())
                        flags += " ";
                    flags += "-L" + std::string(argv[++i]);
                } else {
                    hp::printlnCl("Error: Library path not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }

            } else if (arg.rfind("-L", 0) == 0 && arg.size() > 2) {
                std::string libPath = std::string(arg).substr(2);
                if (!flags.empty())
                    flags += " ";
                flags += "-L" + libPath;

            } else if (arg == "-f" || arg == "--file") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    filename = std::string(argv[++i]);
                } else {
                    hp::printlnCl("Error: filename not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }

            } else if (arg == "--config") {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    selectedConfigs.push_back(std::string(argv[++i]));
                } else {
                    hp::printlnCl("Error: Config name not specified after '" + std::string(arg) + "'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
            } else if (arg.rfind("--config=", 0) == 0 && arg.size() > 9) {
                std::string configName = std::string(arg).substr(9);
                selectedConfigs.push_back(configName);

            } else if (arg.rfind("--config-all", 0) == 0 && arg.size() > 9) {
                allCfgs = true;
            } else if (arg.rfind("-f", 0) == 0 && arg.size() > 2) {
                filename = std::string(arg).substr(2);
            } else if (arg == "-v" || arg == "--verbose") {
                verbose = true;
            } else if (arg == "-r" || arg == "--run") {
                run = true;
            } else if (arg == "--debug") {
                debug = true;
            } else if (arg == "--nofile") {
                n_file = true;
            } else if (arg == "--create") {
                create = true;
            } else if (arg == "--json") {
                json = true;
            } else if (arg == "--no-sep") {
                seperate = false;
            } else if (arg == "--rebuild") {
                rebuild = true;
            } else if (!arg.empty() && (arg.back() == '/' || arg.back() == '\\')) {
                filename = std::string(arg) + "HelpMake.txt";
                if (verbose)
                    std::cout << "Build file: " << filename << std::endl;

            } else if (arg.find(".txt") != std::string::npos && std::filesystem::is_regular_file(arg)) {
                filename = std::string(arg);
                if (verbose)
                    std::cout << "Build file: " << filename << std::endl;

            } else if (std::filesystem::is_directory(arg)) {
                filename = std::string(arg) + "/HelpMake.txt";
                if (verbose)
                    std::cout << "Build file: " << filename << std::endl;

            } else if (arg.rfind("-", 0) != 0) {
                if (!inputFile.empty())
                    inputFile += " ";
                inputFile += std::string(arg);
            }
        }

        if (!std::filesystem::exists(filename) && !n_file && !create) {
            if (filename != "HelpMake.txt" && std::filesystem::exists("HelpMake.txt")) {
                hp::printlnCl("Build file not found. Using default HelpMake.txt", hp::Color::YELLOW);
                filename = "HelpMake.txt";
            }
        }

        if (std::filesystem::exists(filename) && !n_file && !create) {
            Parser parser(filename, inputFile, compiler, version, output, flags, verbose, run, debug, n_file, sere, create, allCfgs, json, seperate, rebuild);
            parser.parse();
            if (!create) {
                parser.executeConfigs(selectedConfigs);
                if (!selectedConfigs.empty() && allCfgs) {
                    hp::printlnCl("Warning: Both --config and --config-all specified. Ignoring --config.", hp::Color::YELLOW);
                }
                if (selectedConfigs.empty()) {
                    parser.execute();
                }
            }
        } else {
            if (!n_file && !create) {
                hp::printlnCl("Build file not found: " + filename, hp::Color::YELLOW);
            }
            if (verbose)
                hp::printlnCl("Building from command-line arguments only\n", hp::Color::CYAN);

            if (compiler.empty()) {
                hp::printlnCl("Error: No compiler specified. Use -Gcc, -Clang, -MSVC, -Zig or Sere.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            if (inputFile.empty()) {
                hp::printlnCl("Error: No input files specified.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            if (output.empty()) {
                std::string d_output;
#ifdef _WIN32
                d_output = "a.exe";
                output = "a.exe";
#else
                d_output = "a.out";
                output = "a.out";
#endif
                hp::printlnCl("Warning: No output file specified. Using default: " + d_output, hp::Color::YELLOW);
            }

            Parser parser(filename, inputFile, compiler, version, output, flags, verbose, run, debug, n_file, sere, create, allCfgs, json, seperate, rebuild);
            parser.parse();
            if (!create)
                parser.execute();
        }
    } else {
        hp::printlnCl("Error: Invalid command. Use \"--help\" for available commands.", hp::Color::RED);
    }

    return 0;
}
