#include "parser.hpp"
#include "hp/help.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

void Parser::parse() {
    if (!n_file && verbose)
        std::cout << "Trying to open file : " << filename.string() << std::endl;

    if (n_file) {
        Parser::buildCommand();
        return;
    }
    std::fstream file(filename);
    if (!file.is_open()) {
        hp::printlnCl("Error: Could not open file: " + filename.string() + "\n", hp::Color::RED);
        hp::printlnCl("Make sure the file exists in the current directory.", hp::Color::YELLOW);
        hp::printlnCl("Or provide all required arguments on the command line", hp::Color::YELLOW);
        exit(EXIT_FAILURE);
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;

        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }

        if (line.find("Compiler:") != std::string::npos) {
            size_t pos = line.find("Compiler:");
            std::string value = line.substr(pos + 9);

            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (compiler.empty()) {
                compiler = value;
            }

            if (compiler.empty()) {
                hp::printlnCl("Error: Compiler not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            } else if (compiler != "gcc" && compiler != "clang" && compiler != "msvc" && compiler != "zig") {
                hp::printlnCl("Error: Unsupported compiler specified: " + compiler, hp::Color::RED);
                hp::printlnCl("Supported compilers are: gcc, clang, msvc and zig.", hp::Color::YELLOW);
                exit(EXIT_FAILURE);
            }

            if (verbose)
                std::cout << "Founded Compiler: '" << compiler << "'" << std::endl;
        } else if (line.find("Version:") != std::string::npos) {
            size_t pos = line.find("Version:");
            std::string value = line.substr(pos + 8);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            version = value;

            if (version.empty()) {
                hp::printlnCl("Error: Version not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            } else if (version != "std=c++98" && version != "std=c++11" &&
                       version != "std=c++14" && version != "std=c++17" &&
                       version != "std=c++20" && version != "std=c++23" &&
                       version != "std=c++26") {
                hp::printlnCl("Error: Unsupported version specified: " + version, hp::Color::RED);
                hp::printlnCl("Supported versions are: std=c++98, std=c++11, std=c++14, std=c++17, std=c++20, std=c++23, std=c++26.", hp::Color::YELLOW);
                exit(EXIT_FAILURE);
            }
            if (verbose)
                std::cout << "Founded Version: '" << version << "'" << std::endl;
        } else if (line.find("Output:") != std::string::npos) {
            size_t pos = line.find("Output:");
            std::string value = line.substr(pos + 7);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (output.empty())
                output = value;

            if (output.empty()) {
                hp::printlnCl("Output not specified in the file. Using default: a.exe", hp::Color::YELLOW);
                output = "a.exe";
            }
            if (verbose)
                std::cout << "Founded Output: '" << output << "' \n";
        } else if (line.find("InputFiles {") != std::string::npos) {
            isInputFileSet = true;
            continue;
        } else if (line.find("}") != std::string::npos && isInputFileSet) {
            isInputFileSet = false;
            continue;
        } else if (isInputFileSet) {
            std::size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos);
                value.erase(value.find_last_not_of(" \t") + 1);
                inputFile += value;
                if (verbose)
                    std::cout << "Founded Input Files: '" << inputFile << "'" << std::endl;
            }
        } else if (line.find("Includes {") != std::string::npos) {
            isIncludeSet = true;
            continue;
        } else if (line.find("}") != std::string::npos && isIncludeSet) {
            isIncludeSet = false;
            if (verbose) {
                std::cout << "Founded Include Files: \n";
                hp::printlnAll(includeFiles);
            }
            continue;
        } else if (isIncludeSet) {
            std::size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos);
                value.erase(value.find_last_not_of(" \t") + 1);
                flags += " -I" + value;
                includeFiles.push_back(value);
            }
        } else if (line.find("Flags:") != std::string::npos) {
            size_t pos = line.find("Flags:");
            std::string value = line.substr(pos + 6);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            flags += " " + value;
            if (verbose)
                std::cout << "Founded Flags: '" << flags << "'" << std::endl;
        } else if (line.find("Github {") != std::string::npos) {
            isGithubSet = true;
            continue;
        } else if (line.find("}") != std::string::npos && isGithubSet) {
            isGithubSet = false;
        } else if (isGithubSet) {
            std::size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos);
                value.erase(value.find_last_not_of(" \t") + 1);
                std::size_t arrow = value.find("->");
                std::string url = value.substr(0, arrow);
                std::string folder = value.substr(arrow + 2);

                url.erase(url.find_last_not_of(" \t\r\n") + 1);
                folder.erase(0, folder.find_first_not_of(" \t\r\n"));
                folder.erase(folder.find_last_not_of(" \t\r\n") + 1);
                std::string test = hp::command("where git");
                if (test == "INFO: Could not find files for the given pattern(s).") {
                    hp::printlnCl("Error: Git system not found. Please ensure Git is installed and added to the system PATH.\n", hp::Color::RED);
                    exit(EXIT_FAILURE);
                }
                hp::Folder f;
                if (!f.exists("build/HelpMake/dep")) {
                    f.create("build/HelpMake/dep");
                }
                if (verbose)
                    std::cout << "[Help_Make] Downloading dependecies '" << url << "' ...\n";
                std::size_t slash = url.find_last_of("/");
                std::string include = url.substr(slash + 1);
                include.erase(include.find_last_not_of(" \t\r\n") + 1);
                if (include.size() > 4 && include.substr(include.size() - 4) == ".git") {
                    include.erase(include.size() - 4);
                }
                std::string cmd = hp::command("git clone " + url + " build/HelpMake/dep/" + include);
                std::string full_path = "build/HelpMake/dep/" + include;
                if (folder != "." && !folder.empty()) {
                    full_path += "/" + folder;
                }
                flags += " -I" + full_path;
                if (debug) {
                    hp::printlnCl("[Debug] Github Folder include of " + value.substr(0, value.size() - 5) + ": '" + folder + "'", hp::YELLOW);
                    hp::printlnCl("[Debug] Github Clone  Folder  of " + value.substr(0, value.size() - 5) + ": '" + include + "'", hp::YELLOW);
                }
            }
        }
    }

    file.close();

    if (compiler.empty()) {
        hp::printlnCl("Error: Compiler not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (version.empty()) {
        hp::printlnCl("Version not specified in the file. Using default: c++20", hp::Color::YELLOW);
        version = "std=c++20";
    }
    if (output.empty()) {
        hp::printlnCl("Output not specified in the file. Using default: a.exe", hp::Color::YELLOW);
        output = "a.exe";
    }
    if (inputFile.empty()) {
        hp::printlnCl("Error: InputFile not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
}

void Parser::execute() {
    std::string comp;
    if (compiler == "gcc") {
        comp = "g++";
    } else if (compiler == "clang") {
        comp = "clang++";
    } else if (compiler == "msvc") {
        comp = "cl";
    } else if (compiler == "zig") {
        comp = "zig c++";
    } else {
        hp::printlnCl("Error: Unsupported compiler specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    std::string command = comp + " -" + version + " " + inputFile + " -o " + output + " " + flags + (run ? " && " + output : "");
    if (verbose)
        std::cout << "\nCommand: " << command << "\n";
    std::string result = hp::command(command);
    if (result.empty()) {
        hp::printlnCl("Compilation successful. Output file: " + output, hp::Color::GREEN);
    } else {
        std::string test = hp::command("where " + (compiler == "zig" ? "zig" : comp));
        if (test == "INFO: Could not find files for the given pattern(s).\n") {

            if (compiler == "gcc") {
                hp::printlnCl("Error: GCC compiler not found. Please ensure GCC is installed and added to the system PATH.\n", hp::Color::RED);
            } else if (compiler == "clang") {
                hp::printlnCl("Error: Clang compiler not found. Please ensure Clang is installed and added to the system PATH.\n", hp::Color::RED);
            } else if (compiler == "msvc") {
                hp::printlnCl("Error: MSVC compiler not found. Please ensure MSVC is installed and added to the system PATH.\n", hp::Color::RED);
            } else if (compiler == "zig") {
                hp::printlnCl("Error: Zig compiler not found. Please ensure Zig is installed and added to the system PATH.\n", hp::Color::RED);
            }
        } else {
            std::cout << result << "\n";
            hp::printlnCl("Compilation Failed", hp::RED);
        }
        exit(EXIT_FAILURE);
    }
}

void Parser::buildCommand() {
    if (compiler.empty()) {
        hp::printlnCl("Error: No compiler specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (inputFile.empty()) {
        hp::printlnCl("Error: No input files specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (output.empty()) {
        hp::printlnCl("Warning: No output specified. Using a.exe", hp::Color::YELLOW);
        output = "a.exe";
    }
    if (version.empty()) {
        version = "std=c++20";
        hp::printlnCl("No version specified, using default: C++20", hp::YELLOW);
    }
    if (verbose) {
        std::cout << "Compiler:    " << hp::getColorCode(hp::YELLOW) << compiler << "\n"
                  << hp::getColorCode(hp::RESET);
        std::cout << "Version:     " << hp::getColorCode(hp::YELLOW) << version << "\n"
                  << hp::getColorCode(hp::RESET);
        std::cout << "Input Files: " << hp::getColorCode(hp::YELLOW) << inputFile << "\n"
                  << hp::getColorCode(hp::RESET);
        std::cout << "Output:      " << hp::getColorCode(hp::YELLOW) << output << "\n"
                  << hp::getColorCode(hp::RESET);
        std::cout << "Flags:      " << hp::getColorCode(hp::YELLOW) << flags << "\n"
                  << hp::getColorCode(hp::RESET);
    }
    execute();
    hp::exit();
}