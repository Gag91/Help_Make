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
        std::cout << "Avaible Flags:\n";
        std::cout << "  -I<path>     Include a specified directory\n";
        std::cout << "  -F<flags>    Add specified flags\n";
        std::cout << "  -L<library>  include a specified library\n";
        std::cout << "Avaible Versions:\n";
        std::cout << "  -std=c++98\n";
        std::cout << "  -std=c++11\n";
        std::cout << "  -std=c++14\n";
        std::cout << "  -std=c++17\n";
        std::cout << "  -std=c++20\n";
    }

    void version() const {
        std::cout << "Help_Make (hm) - Build System\n";
        std::cout << "Version: 1.0.0\n";
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
    Build build;
    if (argc < 2) {
        build.help();
    }
    std::string_view argument = argv[1];

    if (argument == "--help" || argument == "-h") {
        build.help();
    }

    if (argument == "--version" || argument == "-v") {
        build.version();
    }

    if (argument == "--license" || argument == "-l") {
        build.license(hp::EXTENDED);
    }

    if (argument == "--build" || argument == "-b") {
        if (!std::filesystem::exists("HelpMake.txt")) {
            hp::printlnCl("Error: HelpMake.txt file not found in the current directory.", hp::Color::RED);
            exit(EXIT_FAILURE);
        }

        for (int i = 2; i < argc; i++) {
            std::string_view arg = argv[i];
            if (arg == "-Gcc" || arg == "-gcc") {
                compiler = "gcc";
            } else if (arg == "-Clang" || arg == "-clang") {
                compiler = "clang";
            } else if (arg == "-MSVC" || arg == "-msvc") {
                compiler = "msvc";
            } else if (arg == "-std=c++98" || arg == "-std=c++11" || arg == "-std=c++14" ||
                       arg == "-std=c++17" || arg == "-std=c++20" || arg == "-std=c++23" ||
                       arg == "-std=c++26") {
                version = std::string(arg).substr(1);
            } else if (arg.rfind("-o", 0) == 0) {
                std::string n_output = std::string(arg).substr(2);
                if (n_output.empty()) {
                    hp::printlnCl("Error: Output file not specifie after '-o'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
                output = n_output;
            } else if (arg.rfind("-F", 0) == 0) {
                std::string n_flags = std::string(arg).substr(2);
                if (n_flags.empty()) {
                    hp::printlnCl("Error: Additional flags not specified after '-F'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
                flags += " " + n_flags;
            } else if (arg.rfind("-", 0) != 0) {
                std::string n_inputFile = std::string(arg);
                if (n_inputFile.empty()) {
                    hp::printlnCl("Error: Input files not specified after '-'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
                inputFile += " " + n_inputFile;
            } else if ((arg.rfind("-I", 0) == 0) || (arg.rfind("--include", 0) == 0)) {
                std::string includeFlag = std::string(arg).substr(2);
                if (includeFlag.empty()) {
                    hp::printlnCl("Error: Include path not specified after '-I'.", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
                flags += " -I" + includeFlag;

            } else {
                hp::printlnCl("Error: Unknown argument: " + std::string(arg), hp::Color::RED);
                exit(EXIT_FAILURE);
            }
        }
        Parser parser("HelpMake.txt", inputFile, compiler, version, output, flags);
        parser.parse();
        parser.execute();
    }
}
