#include "hp/help.hpp"
#include "parser.hpp"
#include <filesystem>
#include <fstream>

class Build {
  private:
    hp::BorderChars chars = hp::getBorderChars(hp::EXTENDED);

  public:
    Build() = default;
    void help() const {
        std::cout << "Help_Make (hm) - Build System\n";
        std::cout << "Use : hm [options]\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help     Output this help message\n";
        std::cout << "  -v, --version  Output the program version\n";
        std::cout << "  -b, --build    Build HelpMake.txt\n";
        std::cout << "  -l, --license  Output the license information\n\n";
        std::cout << "Avaible Compilers:\n";
        std::cout << "  -Gcc   - GNU Compiler Collection\n";
        std::cout << "  -Clang - LLVM Compiler\n";
        std::cout << "  -MSVC  - Microsoft Visual C++ Compiler\n\n";
        std::cout << "  -Zig   - Zig Compiler (C/C++ support)\n\n";
        std::cout << "Avaible Flags:\n";
        std::cout << "  -I<path>     Include a specified directory\n";
        std::cout << "  -F<flags>    Add specified flags\n";
        std::cout << "  -L<library>  include a specified library\n";
        std::cout << "  -r --run     run output file if compiled\n";
        std::cout << "Avaible Versions:\n";
        std::cout << "  -std=c++98\n";
        std::cout << "  -std=c++11\n";
        std::cout << "  -std=c++14\n";
        std::cout << "  -std=c++17\n";
        std::cout << "  -std=c++20\n";
        std::cout << "  -std=c++23\n";
        std::cout << "  -std=c++26\n";
    }

    void version() const {
        std::cout << "Help_Make (hm) - Build System\n";
        std::cout << "Version: 1.1.0\n";
        std::cout << "Author : Xavi99\n";
        std::cout << "Website: https://github.com/Xavi99/Help_Make\n";
        std::cout << "This program is licensed under the MIT License. See \"--license\" for details.\n\n";
    }
    void license(hp::BorderStyle style) const {
        hp::BorderChars chars = hp::getBorderChars(style);
        std::cout << "Help_Make (hm) - License Information\n";
        std::string license = R"(MIT License

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
            } else if (arg == "-v" || arg == "--verbose") {
                verbose = true;
            } else if (arg == "-r" || arg == "--run") {
                run = true;
            } else if (arg.rfind("/", 0) == 0 || arg.rfind(".", 0) != 0) {
                std::string path = std::string(arg);

                if (!path.empty() && (path.back() == '/' || path.back() == '\\')) {
                    filename = path + "HelpMake.txt";
                } else if (path.find('.') != std::string::npos) {
                    filename = path;
                } else if (std::filesystem::is_directory(path)) {
                    filename = path + "/HelpMake.txt";
                } else {
                    inputFile += " " + path;
                }
            } else if (arg.rfind("-", 0) != 0) {
                if (!inputFile.empty())
                    inputFile += " ";
                inputFile += std::string(arg);
            } else {
                hp::printlnCl("Error: Unknown argument: " + std::string(arg), hp::Color::RED);
                exit(EXIT_FAILURE);
            }
        }

        if (!std::filesystem::exists(filename)) {
            if (filename != "HelpMake.txt" && std::filesystem::exists("HelpMake.txt")) {
                hp::printlnCl("Build file not found. Using default HelpMake.txt", hp::Color::YELLOW);
                filename = "HelpMake.txt";
            }
        }

        if (std::filesystem::exists(filename)) {
            Parser parser(filename, inputFile, compiler, version, output, flags, verbose, run);
            parser.parse();
            parser.execute();
        } else {
            hp::printlnCl("Build file not found: " + filename, hp::Color::YELLOW);
            hp::printlnCl("Building from command-line arguments only", hp::Color::CYAN);

            if (compiler.empty()) {
                hp::printlnCl("Error: No compiler specified. Use -Gcc, -Clang, -MSVC, or -Zig.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            if (inputFile.empty()) {
                hp::printlnCl("Error: No input files specified.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            if (output.empty()) {
                hp::printlnCl("Warning: No output file specified. Using default: a.exe", hp::Color::YELLOW);
                output = "a.exe";
            }

            Parser parser(filename, inputFile, compiler, version, output, flags, verbose, run);
            parser.parse();
            parser.execute();
        }
    } else {
        hp::printlnCl("Error: Invalid command. Use \"--help\" for available commands.", hp::Color::RED);
    }

    return 0;
}