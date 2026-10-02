#include "parser.hpp"
#include "hp/colors/color.hpp"
#include "hp/containers/containers.hpp"
#include "hp/other/other.hpp"
#include "hp/serializer/serialize.hpp"
#include "hp/string/string.hpp"
#include "hp/system/command.hpp"
#include "hp/time/time.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

static std::string trim(const std::string &s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string extractValue(const std::string &line, const std::string &key) {
    std::size_t pos = line.find(key);
    if (pos == std::string::npos)
        return "";
    return trim(line.substr(pos + key.size()));
}

static bool gitAvailable() {
#ifdef _WIN32
    return hp::command("where git").find("INFO:") == std::string::npos;
#else
    return std::system("which git >/dev/null 2>&1") == 0;
#endif
}

static std::string compilerCommand(const std::string &compiler) {
    if (compiler == "gcc")
        return "g++";
    if (compiler == "clang")
        return "clang++";
    if (compiler == "msvc")
        return "cl";
    if (compiler == "zig")
        return "zig c++";
    if (compiler == "sere")
        return "sere";
    return "";
}

void Parser::parse() {

    if (create) {
        hp::File file;

        std::istringstream issInput(inputFile);
        std::string token;
        while (issInput >> token)
            v_inputFiles.push_back(token);

        std::istringstream issFlags(flags);
        while (issFlags >> token)
            v_Flags.push_back(token);

        std::istringstream issModules(modules);
        while (issModules >> token)
            v_Modules.push_back(token);

        if (file.exists(filename.string())) {
            hp::printlnCl(std::format("Warning: {} already exists. Overwrite? (y/n)", filename.string()), hp::Color::YELLOW);
            char verif = hp::get<char>("", "Invalid choice");
            if (verif != 'y') {
                hp::printlnCl("File was not overwritten.", hp::Color::GREEN);
                return;
            }
        }

        hp::save(filename.string(), {{"Compiler", compiler}, {"Version", version}, {"Output", output}});
        std::ofstream make(filename.string(), std::ios::app);

        make << "\nInputFiles {\n";
        for (const auto &input : v_inputFiles)
            make << "    " << input << "\n";
        make << "}\n";

        if (!v_Flags.empty()) {
            make << "\nFlags {\n";
            for (const auto &f : v_Flags)
                make << "    " << f << "\n";
            make << "}\n";
        }
        if (!v_Github.empty()) {
            make << "\nGithub {\n";
            for (const auto &g : v_Github)
                make << "    " << g << "\n";
            make << "}\n";
        }
        if (!v_Modules.empty()) {
            make << "\nModules {\n";
            for (const auto &m : v_Modules)
                make << "    " << m << "\n";
            make << "}\n";
        }
        if (!v_Precmd.empty()) {
            make << "\nPreBuild {\n";
            for (const auto &cmd : v_Precmd)
                make << "    " << cmd << "\n";
            make << "}\n";
        }
        if (!v_Postcmd.empty()) {
            make << "\nPostBuild {\n";
            for (const auto &cmd : v_Postcmd)
                make << "    " << cmd << "\n";
            make << "}\n";
        }

        make.close();
        hp::printlnCl(std::format("{} was created successfully.", filename.string()), hp::Color::GREEN);
        return;
    }

    if (buildSere || compiler == "sere") {
        execute();
        return;
    }

    if (!n_file && verbose)
        std::cout << std::format("[HelpMake]Trying to open file: {}\n", filename.string());

    if (n_file) {
        buildCommand();
        return;
    }

    std::fstream file(filename);
    if (!file.is_open()) {
        hp::printlnCl(std::format("Error: Could not open file: {}\n", filename.string()), hp::Color::RED);
        hp::printlnCl("Make sure the file exists in the current directory.", hp::Color::YELLOW);
        hp::printlnCl("Or provide all required arguments on the command line", hp::Color::YELLOW);
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::string currentBlock;

    while (std::getline(file, line)) {

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (isConfigSet) {
            for (char c : line) {
                if (c == '{')
                    configBraceDepth++;
                else if (c == '}')
                    configBraceDepth--;
            }

            if (configBraceDepth <= 0) {
                configs[currentConfig.name] = currentConfig;
                if (verbose)
                    std::cout << std::format("Saved Config: '{}'\n", currentConfig.name);
                isConfigSet = false;
                currentConfig = Config{};
                currentBlock.clear();
                if (allCfgs)
                    executeConfigs({configs.rbegin()->first});
                continue;
            }

            if (!currentBlock.empty()) {
                if (line.find("}") != std::string::npos) {
                    currentBlock.clear();
                    continue;
                }
                std::string value = trim(line);
                if (value.empty())
                    continue;

                if (currentBlock == "InputFiles") {
                    currentConfig.v_inputFiles.push_back(value);
                    if (!currentConfig.inputFile.empty())
                        currentConfig.inputFile += " ";
                    currentConfig.inputFile += value;

                } else if (currentBlock == "Includes") {
                    currentConfig.includeFiles.push_back(value);

                } else if (currentBlock == "Flags") {
                    currentConfig.v_Flags.push_back(value);
                    if (!currentConfig.flags.empty())
                        currentConfig.flags += " ";
                    currentConfig.flags += value;

                } else if (currentBlock == "Github") {
                    std::string path = processGithubEntry(value);
                    if (!path.empty()) {
                        currentConfig.flags += std::format(" -I{}", path);
                        currentConfig.github.push_back(std::format("{} -> {}", value.substr(0, value.find("->")), path));
                    }

                } else if (currentBlock == "Modules") {
                    if (!currentConfig.modules.empty())
                        currentConfig.modules += " ";
                    currentConfig.modules += value;

                } else if (currentBlock == "PreBuild") {
                    currentConfig.preBuild.push_back(value);

                } else if (currentBlock == "PostBuild") {
                    currentConfig.postBuild.push_back(value);
                }
                continue;
            }

            if (line.find("InputFiles {") != std::string::npos) {
                currentBlock = "InputFiles";
                continue;
            }
            if (line.find("Includes {") != std::string::npos) {
                currentBlock = "Includes";
                continue;
            }
            if (line.find("Flags {") != std::string::npos) {
                currentBlock = "Flags";
                continue;
            }
            if (line.find("Github {") != std::string::npos) {
                currentBlock = "Github";
                continue;
            }
            if (line.find("Modules {") != std::string::npos) {
                currentBlock = "Modules";
                continue;
            }
            if (line.find("PreBuild {") != std::string::npos) {
                currentBlock = "PreBuild";
                continue;
            }
            if (line.find("PostBuild {") != std::string::npos) {
                currentBlock = "PostBuild";
                continue;
            }

            std::string value;
            if (line.find("Compiler:") != std::string::npos) {
                value = extractValue(line, "Compiler:");
                currentConfig.compiler = value;
                if (verbose)
                    std::cout << std::format("Compiler found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Version:") != std::string::npos) {
                value = extractValue(line, "Version:");
                currentConfig.version = value;
                if (verbose)
                    std::cout << std::format("Version found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Output:") != std::string::npos) {
                value = extractValue(line, "Output:");
                currentConfig.output = value;
                if (verbose)
                    std::cout << std::format("Output found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Flags:") != std::string::npos) {
                value = extractValue(line, "Flags:");
                if (currentConfig.flags.empty())
                    currentConfig.flags = value;
                else
                    currentConfig.flags += " " + value;
                currentConfig.v_Flags.push_back(value);
                if (verbose)
                    std::cout << std::format("Flag found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("InputFiles:") != std::string::npos) {
                value = extractValue(line, "InputFiles:");
                if (currentConfig.inputFile.empty())
                    currentConfig.inputFile = value;
                else
                    currentConfig.inputFile += " " + value;
                currentConfig.v_inputFiles.push_back(value);
                if (verbose)
                    std::cout << std::format("Input file found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Includes:") != std::string::npos) {
                value = extractValue(line, "Includes:");
                if (currentConfig.includes.empty())
                    currentConfig.includes = value;
                else
                    currentConfig.includes += " " + value;
                currentConfig.includeFiles.push_back(value);
                if (verbose)
                    std::cout << std::format("Include file found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Run:") != std::string::npos) {
                currentConfig.run = true;
                if (verbose)
                    std::cout << std::format("Run command found in config '{}'\n", currentConfig.name);
            }
            continue;
        }

        if (line.find("Compiler:") != std::string::npos) {
            std::string value = extractValue(line, "Compiler:");
            if (compiler.empty())
                compiler = value;

            if (compiler.empty()) {
                hp::printlnCl("Error: Compiler not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }
            if (compiler != "gcc" && compiler != "clang" && compiler != "msvc" && compiler != "zig" && compiler != "sere") {
                hp::printlnCl(std::format("Error: Unsupported compiler specified: {}", compiler), hp::Color::RED);
                hp::printlnCl("Supported compilers are: gcc, clang, msvc, zig and sere.", hp::Color::YELLOW);
                exit(EXIT_FAILURE);
            }

            if (verbose)
                std::cout << std::format("Founded Compiler: '{}'\n", compiler);
        } else if (line.find("Version:") != std::string::npos) {
            version = extractValue(line, "Version:");
            if (version.empty()) {
                hp::printlnCl("Error: Version not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }
            if (version != "std=c++98" && version != "std=c++11" && version != "std=c++14" &&
                version != "std=c++17" && version != "std=c++20" && version != "std=c++23" && version != "std=c++26") {
                hp::printlnCl(std::format("Error: Unsupported version specified: {}", version), hp::Color::RED);
                hp::printlnCl("Supported versions are: std=c++98, std=c++11, std=c++14, std=c++17, std=c++20, std=c++23, std=c++26.", hp::Color::YELLOW);
                exit(EXIT_FAILURE);
            }
            if (verbose)
                std::cout << std::format("Founded Version: '{}'\n", version);
        } else if (line.find("Output:") != std::string::npos) {
            std::string value = extractValue(line, "Output:");
            if (output.empty())
                output = value;
            if (output.empty()) {
#ifdef _WIN32
                output = "a.exe";
#else
                output = "a.out";
#endif
                hp::printlnCl(std::format("Warning: No output file specified. Using default: {}", output), hp::Color::YELLOW);
            }
            if (verbose)
                std::cout << std::format("Founded Output: '{}'\n", output);
        } else if (line.find("InputFiles:") != std::string::npos) {
            std::string value = extractValue(line, "InputFiles:");
            inputFile += (inputFile.empty() ? "" : " ") + value;
        } else if (line.find("InputFiles {") != std::string::npos) {
            isInputFileSet = true;
        } else if (line.find("}") != std::string::npos && isInputFileSet) {
            isInputFileSet = false;
        } else if (isInputFileSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                inputFile += (inputFile.empty() ? "" : " ") + value;
                v_inputFiles.push_back(value);
                if (verbose)
                    std::cout << std::format("Founded Input Files: '{}'\n", inputFile);
            }

        } else if (line.find("Includes:") != std::string::npos) {
            std::string value = extractValue(line, "Includes:");
            flags += (flags.empty() ? "" : " ") + value;
        } else if (line.find("Includes {") != std::string::npos) {
            isIncludeSet = true;
        } else if (line.find("}") != std::string::npos && isIncludeSet) {
            isIncludeSet = false;
            if (verbose) {
                std::cout << "Founded Include Files:\n";
                for (const auto &vec : includeFiles)
                    std::cout << std::format("- {}\n", vec == "." ? "[root]" : vec);
            }

        } else if (isIncludeSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                flags += std::format(" -I{}", value);
                includeFiles.push_back(value);
            }
        } else if (line.find("Flags {") != std::string::npos) {
            isFlagsSet = true;
        } else if (line.find("}") != std::string::npos && isFlagsSet) {
            isFlagsSet = false;
            if (verbose) {
                std::cout << "Founded Flags:\n";
                for (const auto &vec : v_Flags)
                    std::cout << std::format("- {}\n", vec);
            }
        } else if (isFlagsSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                flags += (flags.empty() ? "" : " ") + value;
                v_Flags.push_back(value);
            }
        } else if (line.find("Flags:") != std::string::npos) {
            flags += " " + extractValue(line, "Flags:");
            if (verbose)
                std::cout << std::format("Founded Flags: '{}'\n", flags);

        } else if (line.find("Github {") != std::string::npos) {
            isGithubSet = true;
        } else if (line.find("}") != std::string::npos && isGithubSet) {
            isGithubSet = false;
        } else if (isGithubSet) {
            std::string value = trim(line);
            if (value.empty())
                continue;

            std::string path = processGithubEntry(value);
            if (!path.empty()) {
                flags += std::format(" -I{}", path);
                v_Github.push_back(std::format("{} -> {}", value.substr(0, value.find("->")), path));
            }

        } else if (line.find("Modules:") != std::string::npos) {
            std::string value = extractValue(line, "Modules:");
            if (modules.empty())
                modules = value;
            else
                modules += " " + value;
            if (verbose)
                std::cout << std::format("Founded Modules: {}\n", value);
            if (debug)
                hp::printlnCl(std::format("[Debug] Compiling Modules: {}", value), hp::Color::YELLOW);
        } else if (line.find("Modules {") != std::string::npos) {
            isModulesSet = true;
        } else if (line.find("}") != std::string::npos && isModulesSet) {
            isModulesSet = false;
            if (verbose) {
                std::cout << "Founded Modules:\n";
                for (const auto &m : v_Modules)
                    std::cout << std::format("- {}\n", m);
            }

        } else if (isModulesSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (modules.empty())
                    modules = value;
                else
                    modules += " " + value;
                v_Modules.push_back(value);
            }
        } else if (line.find("PreBuild {") != std::string::npos) {
            isPreBuildSet = true;
        } else if (line.find("}") != std::string::npos && isPreBuildSet) {
            isPreBuildSet = false;
        } else if (isPreBuildSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (Precmd.empty())
                    Precmd = value;
                else
                    Precmd += " && " + value;
                if (verbose)
                    std::cout << std::format("Pre-build command: {}\n", value);
                v_Precmd.push_back(value);
            }
        } else if (line.find("PostBuild {") != std::string::npos) {
            isPostBuildSet = true;
        } else if (line.find("}") != std::string::npos && isPostBuildSet) {
            isPostBuildSet = false;
        } else if (isPostBuildSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (Postcmd.empty())
                    Postcmd = value;
                else
                    Postcmd += " && " + value;
                if (verbose)
                    std::cout << std::format("Post-build command: {}\n", value);
                v_Postcmd.push_back(value);
            }
        } else if (line.find("Config:") != std::string::npos) {
            std::string rest = line.substr(line.find("Config:") + 7);
            std::size_t brace = rest.find('{');
            if (brace == std::string::npos)
                continue;

            std::string name = trim(rest.substr(0, brace));
            if (name.empty()) {
                hp::printlnCl("Error: Config name is empty.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            isConfigSet = true;
            currentConfig = Config{};
            currentConfig.name = name;
            configBraceDepth = 1;
            if (verbose)
                std::cout << std::format("\nFounded Config: '{}'\n", name);
        }
    }

    file.close();

    if (compiler.empty()) {
        hp::printlnCl("Error: Compiler not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (version.empty()) {
        hp::printlnCl("Version not specified in the file. Using default: std=c++20", hp::Color::YELLOW);
        version = "std=c++20";
    }
    if (output.empty()) {
#ifdef _WIN32
        output = "a.exe";
#else
        output = "a.out";
#endif
        hp::printlnCl(std::format("Output not specified in the file. Using default: {}", output), hp::Color::YELLOW);
    }
    if (inputFile.empty()) {
        hp::printlnCl("Error: InputFile not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
}

void Parser::execute() {
    std::string comp = compilerCommand(compiler);
    if (comp.empty()) {
        hp::printlnCl("Error: Unsupported compiler specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }

    if (!Precmd.empty()) {
        if (verbose)
            hp::printlnCl(std::format("Running pre-build command: {}", Precmd), hp::Color::CYAN);
        int error = std::system(Precmd.c_str());
        if (error == -1) {
            hp::printlnCl("Error: Could not launch pre-build command.", hp::Color::RED);
            exit(EXIT_FAILURE);
        }
        if (error != 0) {
            hp::printlnCl(std::format("Error: Pre-build command failed (exit code {}).", error), hp::Color::RED);
            exit(EXIT_FAILURE);
        }
        if (verbose)
            hp::printlnCl("Pre-build command executed successfully.", hp::Color::GREEN);
    }

    std::string command;
    std::string displayCommand;

    if (compiler != "sere") {
        std::string body = version;
        if (!modules.empty())
            body += " " + modules;
        body += " " + inputFile;
        body += " -o " + output;
        if (!flags.empty())
            body += " " + flags;
        if (run)
            body += " && " + output;

        command = std::format("{} -fdiagnostics-color=always -{}", comp, body);
        displayCommand = std::format("{} -{}", comp, body);
    } else {
        std::string body = modules;
        if (!body.empty())
            body += " ";
        body += inputFile + " -o " + output;
        if (!flags.empty())
            body += " " + flags;
        if (run)
            body += " && " + output;

        command = std::format("{} {}", comp, body);
        displayCommand = command;
    }

    if (verbose)
        std::cout << std::format("\nCommand: {}\n", displayCommand);

    std::string r_logDir = "build/HelpMake/logs/raw";
    std::string logDir = "build/HelpMake/logs";
    std::filesystem::create_directories(r_logDir);

    std::size_t dotPos = output.find_last_of('.');
    std::string baseName = (dotPos != std::string::npos) ? output.substr(0, dotPos) : output;

    std::string r_logPath = r_logDir + "/" + baseName + ".txt";
    std::string logPath = logDir + "/" + baseName + ".txt";

    std::string redirectCmd = std::format("{} > \"{}\" 2>&1", command, r_logPath);
    auto timer = hp::startTimer();
    int exitCode = std::system(redirectCmd.c_str());
    double elapsed = hp::stopTimer(timer);

    std::string result;
    std::ifstream rawFile(r_logPath);
    if (rawFile.is_open())
        result.assign((std::istreambuf_iterator<char>(rawFile)), std::istreambuf_iterator<char>());

    std::regex ansi_pattern("\x1B\\[[0-9;]*[a-zA-Z]");
    std::string cleanLog = std::regex_replace(result, ansi_pattern, "");
    std::ofstream cleanFile(logPath, std::ios::trunc);
    if (cleanFile.is_open())
        cleanFile << cleanLog;

    if (exitCode == 0) {
        hp::printlnCl(std::format("Compilation successful. Output file: {}", output), hp::Color::GREEN);
        if (!result.empty() && verbose)
            std::cout << result << "\n";

        if (!Postcmd.empty()) {
            if (verbose)
                hp::printlnCl(std::format("Running post-build command: {}", Postcmd), hp::Color::CYAN);
            int Error = std::system(Postcmd.c_str());
            if (Error == -1)
                hp::printlnCl("Warning: Could not launch post-build command.", hp::Color::YELLOW);
            else if (Error != 0)
                hp::printlnCl(std::format("Warning: Post-build command failed (exit code {}).", Error), hp::Color::YELLOW);
            else if (verbose)
                hp::printlnCl("Post-build command executed successfully.", hp::Color::GREEN);
        }
    } else {
#ifdef _WIN32
        std::string whichCmd = std::format("where {} 2>nul", compiler == "zig" ? "zig" : comp);
        std::string test = hp::command(whichCmd);
        bool missing = test.find("INFO:") != std::string::npos;
#else
        std::string whichCmd = std::format("which {} 2>/dev/null", compiler == "zig" ? "zig" : comp);
        bool missing = std::system(whichCmd.c_str()) != 0;
#endif

        if (missing) {
            hp::printlnCl(std::format("Error: {} compiler not found. Please install it and add it to PATH.", compiler), hp::Color::RED);
        } else {
            if (!cleanLog.empty())
                std::cout << result << "\n";
            hp::printlnCl("\nCompilation Failed", hp::Color::RED);
            hp::printlnCl(std::format("See Logs: {}", logPath), hp::Color::YELLOW);
        }
        exit(EXIT_FAILURE);
    }

    if (verbose)
        std::cout << std::format("Compiling time: {}s\n", elapsed);
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
    if (version.empty() && compiler != "sere") {
        version = "std=c++20";
        hp::printlnCl("No version specified, using default: C++20", hp::Color::YELLOW);
    }

    if (verbose) {
        std::cout << std::format("Compiler:    {}{}\n", hp::getColorCode(hp::YELLOW), compiler)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Version:     {}{}\n", hp::getColorCode(hp::YELLOW), version)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Input Files: {}{}\n", hp::getColorCode(hp::YELLOW), inputFile)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Output:      {}{}\n", hp::getColorCode(hp::YELLOW), output)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Flags:       {}{}\n", hp::getColorCode(hp::YELLOW), flags)
                  << hp::getColorCode(hp::RESET);
    }
    execute();
    hp::exit(0);
}

void Parser::executeConfigs(const std::vector<std::string> &names) {
    for (const auto &name : names) {
        auto it = configs.find(name);
        if (it == configs.end()) {
            if (debug)
                hp::printlnCl(std::format("[Debug] Config name is: '{}'", name), hp::Color::YELLOW);
            hp::printlnCl(std::format("Error: Config '{}' not found.", name), hp::Color::RED);
            hp::printlnCl("Available configs:", hp::Color::YELLOW);
            for (const auto &[n, _] : configs)
                std::cout << std::format("- {}\n", n);
            continue;
        }

        Config cfg = it->second;

        if (cfg.compiler.empty())
            cfg.compiler = compiler;
        if (cfg.version.empty())
            cfg.version = version;
        if (cfg.output.empty())
            cfg.output = output;
        if (cfg.flags.empty())
            cfg.flags = flags;
        if (cfg.inputFile.empty())
            cfg.inputFile = inputFile;
        if (cfg.v_Flags.empty())
            cfg.v_Flags = v_Flags;
        if (cfg.v_inputFiles.empty())
            cfg.v_inputFiles = v_inputFiles;
        if (cfg.includeFiles.empty())
            cfg.includeFiles = includeFiles;

        std::string files;
        for (const auto &f : cfg.v_inputFiles) {
            if (!files.empty())
                files += " ";
            files += f;
        }
        if (files.empty())
            files = cfg.inputFile;

        std::string incFlags;
        for (const auto &inc : cfg.includeFiles)
            incFlags += std::format(" -I{}", inc);

        std::string comp = compilerCommand(cfg.compiler);
        if (comp.empty()) {
            hp::printlnCl(std::format("Error: Unsupported compiler for config '{}': {}", name, cfg.compiler), hp::Color::RED);
            continue;
        }

        hp::printlnCl(std::format("\nBuilding config: {}", name), hp::Color::CYAN);

        std::string command = std::format("{} -{} {} -o {} {}{}",
                                          comp, cfg.version, files, cfg.output, cfg.flags, incFlags);

        if (verbose)
            std::cout << std::format("Command: {}\n", command);

        int result = std::system(command.c_str());
        if (result == 0)
            hp::printlnCl(std::format("Config '{}' compiled successfully. Output: {}", name, cfg.output), hp::Color::GREEN);
        else
            hp::printlnCl(std::format("Config '{}' compilation failed.", name), hp::Color::RED);
    }
}

std::string Parser::processGithubEntry(const std::string &value) {
    std::size_t arrow = value.find("->");
    if (arrow == std::string::npos)
        return "";

    std::string url = trim(value.substr(0, arrow));
    std::string folder = trim(value.substr(arrow + 2));

    if (!gitAvailable()) {
        hp::printlnCl("Error: Git system not found. Please install it and add it to PATH.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }

    hp::Folder f;
    if (!f.exists("build/HelpMake/dep"))
        f.create("build/HelpMake/dep");

    if (verbose)
        hp::printlnCl(std::format("\n[Help_Make] Downloading dependency '{}' ...\n", url), hp::Color::CYAN);

    std::size_t slash = url.find_last_of("/");
    std::string include = url.substr(slash + 1);
    if (include.size() > 4 && include.substr(include.size() - 4) == ".git")
        include.erase(include.size() - 4);

    hp::command(std::format("git clone {} build/HelpMake/dep/{}", url, include));

    std::string fullPath = std::format("build/HelpMake/dep/{}", include);
    if (!folder.empty() && folder != ".")
        fullPath += "/" + folder;

    if (debug) {
        hp::printlnCl(std::format("[Debug] Github Folder include: '{}'", folder), hp::Color::YELLOW);
        hp::printlnCl(std::format("[Debug] Github Clone Folder: '{}'", include), hp::Color::YELLOW);
    }

    return fullPath;
}

std::vector<std::string> Parser::getInputFile() {
    return v_inputFiles;
}
std::vector<std::string> Parser::getInclude() {
    return includeFiles;
}
std::vector<std::string> Parser::getFlags() {
    return v_Flags;
}
std::vector<std::string> Parser::getModules() {
    return v_Modules;
}
std::vector<std::string> Parser::getGithub() {
    return v_Github;
}
std::string Parser::getCompiler() {
    return compiler;
}
std::string Parser::getOutput() {
    return output;
}
std::string Parser::getVersion() {
    return version;
}