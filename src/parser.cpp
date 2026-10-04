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
    if (compiler == "clang")
        return "clang++";
    if (compiler == "msvc")
        return "cl";
    if (compiler == "zig")
        return "zig c++";
    return compiler;
}

void Parser::parse() {

    if (cfg.create) {
        hp::File file;

        cfg.v_inputFiles = splitArgs(cfg.inputFile);
        cfg.v_Flags = splitArgs(cfg.flags);
        cfg.includeFiles = splitArgs(cfg.includes);
        cfg.v_Modules = splitArgs(cfg.modules);

        if (file.exists(filename.string()) && !cfg.quiet) {
            hp::printlnCl(std::format("Warning: {} already exists. Overwrite? (y/n)", filename.string()), hp::Color::YELLOW);
            char verif = hp::get<char>("", "Invalid choice");
            if (verif != 'y') {
                hp::printlnCl("File was not overwritten.", hp::Color::GREEN);
                return;
            }
        }

        hp::save(filename.string(), {{"Compiler", cfg.compiler}, {"Version", cfg.version}, {"Output", cfg.output}});
        std::ofstream make(filename.string(), std::ios::app);

        make << "\nInputFiles {\n";
        for (const auto &input : cfg.v_inputFiles)
            make << "    " << input << "\n";
        make << "}\n";

        if (!cfg.v_Flags.empty()) {
            make << "\nFlags {\n";
            for (const auto &f : cfg.v_Flags)
                make << "    " << f << "\n";
            make << "}\n";
        }
        if (!cfg.v_Github.empty()) {
            make << "\nGithub {\n";
            for (const auto &g : cfg.v_Github)
                make << "    " << g << "\n";
            make << "}\n";
        }
        if (!cfg.v_Modules.empty()) {
            make << "\nModules {\n";
            for (const auto &m : cfg.v_Modules)
                make << "    " << m << "\n";
            make << "}\n";
        }
        if (!cfg.v_Precmd.empty()) {
            make << "\nPreBuild {\n";
            for (const auto &cmd : cfg.v_Precmd)
                make << "    " << cmd << "\n";
            make << "}\n";
        }
        if (!cfg.v_Postcmd.empty()) {
            make << "\nPostBuild {\n";
            for (const auto &cmd : cfg.v_Postcmd)
                make << "    " << cmd << "\n";
            make << "}\n";
        }

        make.close();
        hp::printlnCl(std::format("{} was created successfully.", filename.string()), hp::Color::GREEN);
        return;
    }

    if (cfg.buildSere || cfg.compiler == "sere") {
        execute();
        return;
    }

    if (!cfg.n_file && cfg.verbose)
        std::cout << std::format("[HelpMake] Trying to open file: {}\n", filename.string());

    if (cfg.n_file) {
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

        std::size_t hash = line.find('#');
        if (hash != std::string::npos)
            line = line.substr(0, hash);

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
                if (cfg.verbose)
                    std::cout << std::format("Saved Config: '{}'\n", currentConfig.name);
                isConfigSet = false;
                currentConfig = Config{};
                currentBlock.clear();
                if (cfg.allCfgs)
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

                } else if (currentBlock == "Fetch") {
                    std::string path = processGithubEntry(value);
                    if (!path.empty()) {
                        currentConfig.flags += std::format(" -I{}", path);
                        currentConfig.v_Github.push_back(std::format("{} -> {}", value.substr(0, value.find("->")), path));
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
            if (line.find("Fetch {") != std::string::npos) {
                currentBlock = "Fetch";
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
                if (cfg.verbose)
                    std::cout << std::format("Compiler found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Version:") != std::string::npos) {
                value = extractValue(line, "Version:");
                currentConfig.version = value;
                if (cfg.verbose)
                    std::cout << std::format("Version found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Output:") != std::string::npos) {
                value = extractValue(line, "Output:");
                currentConfig.output = value;
                if (cfg.verbose)
                    std::cout << std::format("Output found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Flags:") != std::string::npos) {
                value = extractValue(line, "Flags:");
                if (currentConfig.flags.empty())
                    currentConfig.flags = value;
                else
                    currentConfig.flags += " " + value;
                currentConfig.v_Flags.push_back(value);
                if (cfg.verbose)
                    std::cout << std::format("Flag found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("InputFiles:") != std::string::npos) {
                value = extractValue(line, "InputFiles:");
                if (currentConfig.inputFile.empty())
                    currentConfig.inputFile = value;
                else
                    currentConfig.inputFile += " " + value;
                currentConfig.v_inputFiles.push_back(value);
                if (cfg.verbose)
                    std::cout << std::format("Input file found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Includes:") != std::string::npos) {
                value = extractValue(line, "Includes:");
                if (currentConfig.includes.empty())
                    currentConfig.includes = value;
                else
                    currentConfig.includes += " " + value;
                currentConfig.includeFiles.push_back(value);
                if (cfg.verbose)
                    std::cout << std::format("Include file found in config '{}': '{}'\n", currentConfig.name, value);

            } else if (line.find("Run:") != std::string::npos) {
                currentConfig.run = true;
                if (cfg.verbose)
                    std::cout << std::format("Run command found in config '{}'\n", currentConfig.name);
            }
            continue;
        }

        if (line.find("Compiler:") != std::string::npos) {
            std::string value = extractValue(line, "Compiler:");
            if (cfg.compiler.empty())
                cfg.compiler = value;

            if (cfg.compiler.empty()) {
                hp::printlnCl("Error: Compiler not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }

            if (cfg.verbose)
                std::cout << std::format("Founded Compiler: '{}'\n", cfg.compiler);
        } else if (line.find("Version:") != std::string::npos) {
            cfg.version = extractValue(line, "Version:");
            if (cfg.version.empty()) {
                hp::printlnCl("Error: Version not specified in the file.", hp::Color::RED);
                exit(EXIT_FAILURE);
            }
            if (cfg.verbose)
                std::cout << std::format("Founded Version: '{}'\n", cfg.version);
        } else if (line.find("Output:") != std::string::npos) {
            std::string value = extractValue(line, "Output:");
            if (cfg.output.empty())
                cfg.output = value;
            if (cfg.output.empty()) {
#ifdef _WIN32
                cfg.output = "a.exe";
#else
                cfg.output = "a.out";
#endif
                hp::printlnCl(std::format("Warning: No output file specified. Using default: {}", cfg.output), hp::Color::YELLOW);
            }
            if (cfg.verbose)
                std::cout << std::format("Founded Output: '{}'\n", cfg.output);
        } else if (line.find("InputFiles:") != std::string::npos) {
            std::string value = extractValue(line, "InputFiles:");
            cfg.inputFile += (cfg.inputFile.empty() ? "" : " ") + value;
        } else if (line.find("InputFiles {") != std::string::npos) {
            isInputFileSet = true;
        } else if (line.find("}") != std::string::npos && isInputFileSet) {
            isInputFileSet = false;
        } else if (isInputFileSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                cfg.inputFile += (cfg.inputFile.empty() ? "" : " ") + value;
                cfg.v_inputFiles.push_back(value);
                if (cfg.verbose)
                    std::cout << std::format("Founded Input Files: '{}'\n", cfg.inputFile);
            }

        } else if (line.find("Includes:") != std::string::npos) {
            std::string value = extractValue(line, "Includes:");
            cfg.flags += (cfg.flags.empty() ? "" : " ") + value;
        } else if (line.find("Includes {") != std::string::npos) {
            isIncludeSet = true;
        } else if (line.find("}") != std::string::npos && isIncludeSet) {
            isIncludeSet = false;
            if (cfg.verbose) {
                std::cout << "Founded Include Files:\n";
                for (const auto &vec : cfg.includeFiles)
                    std::cout << std::format("- {}\n", vec == "." ? "[root]" : vec);
            }

        } else if (isIncludeSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                cfg.flags += std::format(" -I{}", value);
                cfg.includeFiles.push_back(value);
            }
        } else if (line.find("Flags {") != std::string::npos) {
            isFlagsSet = true;
        } else if (line.find("}") != std::string::npos && isFlagsSet) {
            isFlagsSet = false;
            if (cfg.verbose) {
                std::cout << "Founded Flags:\n";
                for (const auto &vec : cfg.v_Flags)
                    std::cout << std::format("- {}\n", vec);
            }
        } else if (isFlagsSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                cfg.flags += (cfg.flags.empty() ? "" : " ") + value;
                cfg.v_Flags.push_back(value);
            }
        } else if (line.find("Flags:") != std::string::npos) {
            cfg.flags += " " + extractValue(line, "Flags:");
            if (cfg.verbose)
                std::cout << std::format("Founded Flags: '{}'\n", cfg.flags);

        } else if (line.find("Fetch {") != std::string::npos) {
            isFetchSet = true;
        } else if (line.find("}") != std::string::npos && isFetchSet) {
            isFetchSet = false;
        } else if (isFetchSet) {
            std::string value = trim(line);
            if (value.empty())
                continue;

            std::string path = processGithubEntry(value);
            if (!path.empty()) {
                cfg.flags += std::format(" -I{}", path);
                cfg.v_Github.push_back(std::format("{} -> {}", value.substr(0, value.find("->")), path));
            }

        } else if (line.find("Modules:") != std::string::npos) {
            std::string value = extractValue(line, "Modules:");
            if (cfg.modules.empty())
                cfg.modules = value;
            else
                cfg.modules += " " + value;
            if (cfg.verbose)
                std::cout << std::format("Founded Modules: {}\n", value);
            if (cfg.debug)
                hp::printlnCl(std::format("[Debug] Compiling Modules: {}", value), hp::Color::YELLOW);
        } else if (line.find("Modules {") != std::string::npos) {
            isModulesSet = true;
        } else if (line.find("}") != std::string::npos && isModulesSet) {
            isModulesSet = false;
            if (cfg.verbose) {
                std::cout << "Founded Modules:\n";
                for (const auto &m : cfg.v_Modules)
                    std::cout << std::format("- {}\n", m);
            }

        } else if (isModulesSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (cfg.modules.empty())
                    cfg.modules = value;
                else
                    cfg.modules += " " + value;
                cfg.v_Modules.push_back(value);
            }
        } else if (line.find("PreBuild {") != std::string::npos) {
            isPreBuildSet = true;
        } else if (line.find("}") != std::string::npos && isPreBuildSet) {
            isPreBuildSet = false;
        } else if (isPreBuildSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (cfg.Precmd.empty())
                    cfg.Precmd = value;
                else
                    cfg.Precmd += " && " + value;
                if (cfg.verbose)
                    std::cout << std::format("Pre-build command: {}\n", value);
                cfg.v_Precmd.push_back(value);
            }
        } else if (line.find("PostBuild {") != std::string::npos) {
            isPostBuildSet = true;
        } else if (line.find("}") != std::string::npos && isPostBuildSet) {
            isPostBuildSet = false;
        } else if (isPostBuildSet) {
            std::string value = trim(line);
            if (!value.empty()) {
                if (cfg.Postcmd.empty())
                    cfg.Postcmd = value;
                else
                    cfg.Postcmd += " && " + value;
                if (cfg.verbose)
                    std::cout << std::format("Post-build command: {}\n", value);
                cfg.v_Postcmd.push_back(value);
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
            if (cfg.verbose)
                std::cout << std::format("\nFounded Config: '{}'\n", name);
        }
    }

    file.close();

    if (cfg.compiler.empty()) {
        hp::printlnCl("Error: Compiler not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (cfg.output.empty()) {
#ifdef _WIN32
        cfg.output = "a.exe";
#else
        cfg.output = "a.out";
#endif
        hp::printlnCl(std::format("Output not specified in the file. Using default: {}", cfg.output), hp::Color::YELLOW);
    }
    if (cfg.inputFile.empty()) {
        hp::printlnCl("Error: InputFile not found in the file.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
}

void Parser::execute() {
    std::string comp = compilerCommand(cfg.compiler);
    if (comp.empty()) {
        hp::printlnCl("Error: Unspecified Compiler.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }

    if (cfg.json) {
        Parser::generateCompileCommands();
    }

    if (!cfg.Precmd.empty()) {
        if (cfg.verbose)
            hp::printlnCl(std::format("Running pre-build command: {}", cfg.Precmd), hp::Color::CYAN);
        int error = std::system(cfg.Precmd.c_str());
        if (error == -1) {
            hp::printlnCl("Error: Could not launch pre-build command.", hp::Color::RED);
            exit(EXIT_FAILURE);
        }
        if (error != 0) {
            hp::printlnCl(std::format("Error: Pre-build command failed (exit code {}).", error), hp::Color::RED);
            exit(EXIT_FAILURE);
        }
        if (cfg.verbose)
            hp::printlnCl("Pre-build command executed successfully.", hp::Color::GREEN);
    }

    if (!cfg.version.empty()) {
        if (cfg.version.back() != '-')
            cfg.version.insert(cfg.version.begin(), '-');
    }

    std::filesystem::create_directories(r_logDir);
    std::string CommandFlags;
    if (!cfg.version.empty())
        CommandFlags = cfg.version;
    if (!cfg.modules.empty())
        CommandFlags += (CommandFlags.empty() ? "" : " ") + cfg.modules;
    if (!cfg.flags.empty())
        CommandFlags += (CommandFlags.empty() ? "" : " ") + cfg.flags;

    std::string displayCommand;
    std::string command;

    int exitCode = 0;
    double elapsed = 0;
    displayCommand = std::format("{} {} {} -o {}", comp, CommandFlags, cfg.inputFile, cfg.output);
    if (cfg.verbose) {
        std::cout << std::format("\nCommand: {}\n", displayCommand);
    }
    auto timer = hp::startTimer();
    if (cfg.seperate) {
        std::filesystem::create_directories("build/HelpMake/obj");

        std::vector<std::string> expandedFiles;
        for (const auto &entry : cfg.v_inputFiles) {
            if (entry.find('*') != std::string::npos) {
                std::filesystem::path p(entry);
                std::string dir = p.parent_path().string();

                std::string extension = p.extension().string();
                if (extension.empty()) {
                    expandedFiles.push_back(entry);
                    continue;
                }

                for (const auto &file : std::filesystem::directory_iterator(dir)) {
                    if (file.is_regular_file() && file.path().extension() == extension)
                        expandedFiles.push_back(file.path().string());
                }
            } else {
                expandedFiles.push_back(entry);
            }
        }

        if (cfg.debug) {
            hp::printlnCl(std::format("[HelpMake] {} files to check:", expandedFiles.size()), hp::Color::YELLOW);
            for (const auto &file : expandedFiles)
                hp::printlnCl(std::format("- {}", file), hp::Color::YELLOW);
        }
        std::cout << '\n';

        std::vector<std::string> objects;

        for (const auto &file : expandedFiles) {
            std::string baseName = std::filesystem::path(file).stem().string();
            std::string objPath = std::format("build/HelpMake/obj/{}.o", baseName);

            if (!needsRebuild(file, objPath)) {
                if (cfg.debug)
                    hp::printlnCl(std::format("[HelpMake] Skip (up to date): {}", file), hp::Color::YELLOW);
                objects.push_back(objPath);
                continue;
            }

            std::string objCmd = std::format("{} -fdiagnostics-color=always {} -c \"{}\" -o \"{}\" -MMD", comp, CommandFlags, file, objPath);

            if (cfg.debug)
                hp::printlnCl(std::format("[HelpMake] Object Command: {}\n", objCmd), hp::Color::YELLOW);

            std::string redirectCmd = std::format("{} > \"{}\" 2>&1", objCmd, r_logPath);
            exitCode = std::system(redirectCmd.c_str());
            objects.push_back(objPath);
        }

        if (exitCode == 0) {

            std::string linkCmd = comp + " -fdiagnostics-color=always " + CommandFlags;
            for (const auto &obj : objects)
                linkCmd += " \"" + obj + "\"";
            linkCmd += " -o \"" + cfg.output + "\"" + (cfg.run ? " && " + cfg.output : "");

            if (cfg.debug)
                hp::printlnCl(std::format("\n[HelpMake] Link Command: {}", linkCmd), hp::Color::YELLOW);

            std::string redirectCmd = std::format("{} > \"{}\" 2>&1", linkCmd, r_logPath);
            exitCode = std::system(redirectCmd.c_str());
        }

    } else {
        displayCommand = std::format("{} -{} {} -o {}", comp, CommandFlags, cfg.inputFile, cfg.output);

        command = std::format("{} -fdiagnostics-color=always {} {} -o {} {}",
                              comp, CommandFlags, cfg.inputFile, cfg.output, (cfg.run ? "&& " + cfg.output : ""));

        if (cfg.debug)
            hp::printlnCl(std::format("[HelpMake] Real Command: {}", command), hp::Color::YELLOW);

        std::string redirectCmd = std::format("{} > \"{}\" 2>&1", command, r_logPath);
        exitCode = std::system(redirectCmd.c_str());
    }
    elapsed = hp::stopTimer(timer);

    std::string result;
    std::ifstream rawFile(r_logPath);
    if (rawFile.is_open())
        result.assign((std::istreambuf_iterator<char>(rawFile)), std::istreambuf_iterator<char>());

    std::regex ansi_pattern("\x1B\\[[0-9;]*[a-zA-Z]");
    std::string cleanLog = std::regex_replace(result, ansi_pattern, "");
    {
        std::ofstream cleanFile(logPath, std::ios::trunc);
        if (cleanFile.is_open())
            cleanFile << cleanLog;
    }

    if (exitCode == 0) {
        hp::printlnCl(std::format("Compilation successful. Output file: {}", cfg.output), hp::Color::GREEN);
        if (!result.empty() && cfg.verbose)
            std::cout << result << "\n";

        if (!cfg.Postcmd.empty()) {
            if (cfg.verbose)
                hp::printlnCl(std::format("Running post-build command: {}", cfg.Postcmd), hp::Color::CYAN);
            int Error = std::system(cfg.Postcmd.c_str());
            if (Error == -1 && !cfg.quiet)
                hp::printlnCl("Warning: Could not launch post-build command.", hp::Color::YELLOW);
            else if (Error != 0 && !cfg.quiet)
                hp::printlnCl(std::format("Warning: Post-build command failed (exit code {}).", Error), hp::Color::YELLOW);
            else if (cfg.verbose)
                hp::printlnCl("Post-build command executed successfully.", hp::Color::GREEN);
        }
    } else {
#ifdef _WIN32
        std::string whichCmd = std::format("where {} 2>nul", cfg.compiler == "zig" ? "zig" : comp);
        std::string test = hp::command(whichCmd);
        bool missing = test.find("INFO:") != std::string::npos;
#else
        std::string whichCmd = std::format("which {} 2>/dev/null", cfg.compiler == "zig" ? "zig" : comp);
        bool missing = std::system(whichCmd.c_str()) != 0;
#endif

        if (missing) {
            hp::printlnCl(std::format("Error: {} compiler not found. Please install it and add it to PATH.", cfg.compiler), hp::Color::RED);
        } else {
            if (!cleanLog.empty())
                std::cout << result << "\n";
            hp::printlnCl("\nCompilation Failed", hp::Color::RED);
            hp::printlnCl(std::format("See Logs: {}", logPath), hp::Color::YELLOW);
        }
        exit(EXIT_FAILURE);
    }

    if (cfg.verbose)
        std::cout << std::format("Compiling time: {}s\n", elapsed);
}

void Parser::buildCommand() {
    if (cfg.compiler.empty()) {
        hp::printlnCl("Error: No compiler specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (cfg.inputFile.empty()) {
        hp::printlnCl("Error: No input files specified.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }
    if (cfg.output.empty()) {
        if (!cfg.quiet)
            hp::printlnCl("Warning: No output specified. Using a.exe", hp::Color::YELLOW);
        cfg.output = "a.exe";
    }

    cfg.v_inputFiles = splitArgs(cfg.inputFile);
    cfg.v_Flags = splitArgs(cfg.flags);
    cfg.includeFiles = splitArgs(cfg.includes);
    cfg.v_Modules = splitArgs(cfg.modules);

    if (cfg.verbose) {
        std::cout << std::format("Compiler:    {}{}\n", hp::getColorCode(hp::YELLOW), cfg.compiler)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Version:     {}{}\n", hp::getColorCode(hp::YELLOW), cfg.version)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Input Files: {}{}\n", hp::getColorCode(hp::YELLOW), cfg.inputFile)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Output:      {}{}\n", hp::getColorCode(hp::YELLOW), cfg.output)
                  << hp::getColorCode(hp::RESET);
        std::cout << std::format("Flags:       {}{}\n", hp::getColorCode(hp::YELLOW), cfg.flags)
                  << hp::getColorCode(hp::RESET);
    }
    execute();
    hp::exit(0);
}

void Parser::executeConfigs(const std::vector<std::string> &names) {
    for (const auto &name : names) {
        auto it = configs.find(name);
        if (it == configs.end()) {
            if (cfg.debug)
                hp::printlnCl(std::format("[Debug] Config name is: '{}'", name), hp::Color::YELLOW);
            hp::printlnCl(std::format("Error: Config '{}' was either not found or is invalid.", name), hp::Color::RED);
            hp::printlnCl("Available configs:", hp::Color::YELLOW);
            for (const auto &[n, _] : configs)
                std::cout << std::format("- {}\n", n);
            continue;
        }

        Config local = it->second;

        if (local.compiler.empty())
            local.compiler = cfg.compiler;
        if (local.version.empty())
            local.version = cfg.version;
        if (local.output.empty())
            local.output = cfg.output;
        if (local.flags.empty())
            local.flags = cfg.flags;
        if (local.inputFile.empty())
            local.inputFile = cfg.inputFile;
        if (local.v_Flags.empty())
            local.v_Flags = cfg.v_Flags;
        if (local.v_inputFiles.empty())
            local.v_inputFiles = cfg.v_inputFiles;
        if (local.includeFiles.empty())
            local.includeFiles = cfg.includeFiles;
        if (local.v_Precmd.empty())
            local.v_Precmd = cfg.v_Precmd;
        if (local.v_Postcmd.empty())
            local.v_Postcmd = cfg.v_Postcmd;
        if (local.Precmd.empty())
            local.Precmd = cfg.Precmd;
        if (local.Postcmd.empty())
            local.Postcmd = cfg.Postcmd;

        std::string files;
        for (const auto &f : local.v_inputFiles) {
            if (!files.empty())
                files += " ";
            files += f;
        }
        if (files.empty())
            files = local.inputFile;

        std::string incFlags;
        for (const auto &inc : local.includeFiles)
            incFlags += std::format(" -I{}", inc);

        std::string comp = compilerCommand(local.compiler);
        if (comp.empty()) {
            hp::printlnCl(std::format("Error: Unsupported compiler '{}'", local.compiler), hp::Color::RED);
            continue;
        }

        hp::printlnCl(std::format("\nBuilding config: {}", name), hp::Color::CYAN);
        std::string command;
        std::string displayCommand;
        std::string CommandFlags;
        displayCommand = std::format("{} {} {} -o {}", comp, CommandFlags, cfg.inputFile, cfg.output);
        if (cfg.verbose) {
            std::cout << std::format("\nConfig '{}': Command: {}\n", name, displayCommand);
        }
        int exitCode = 0;
        if (!local.version.empty())
            CommandFlags = local.version;
        if (!local.modules.empty())
            CommandFlags += (CommandFlags.empty() ? "" : " ") + local.modules;
        if (!local.flags.empty())
            CommandFlags += (CommandFlags.empty() ? "" : " ") + local.flags;
        if (local.seperate) {
            std::vector<std::string> expandedFiles;

            for (const auto &entry : local.v_inputFiles) {
                if (entry.find('*') != std::string::npos) {
                    std::filesystem::path p(entry);
                    std::string dir = p.parent_path().string();

                    std::string extension = p.extension().string();
                    if (extension.empty()) {
                        expandedFiles.push_back(entry);
                        continue;
                    }

                    for (const auto &file : std::filesystem::directory_iterator(dir)) {
                        if (file.is_regular_file() && file.path().extension() == extension)
                            expandedFiles.push_back(file.path().string());
                    }
                } else {
                    expandedFiles.push_back(entry);
                }
            }

            if (cfg.debug) {
                hp::printlnCl(std::format("[HelpMake] Config '{}': {} files to check:", name, expandedFiles.size()), hp::Color::YELLOW);
                for (const auto &file : expandedFiles)
                    hp::printlnCl(std::format("- {}", file), hp::Color::YELLOW);
            }

            std::vector<std::string> objs;
            for (const auto &file : expandedFiles) {
                std::string basename = std::filesystem::path(file).stem().string();
                std::string Obj = std::format("build/HelpMake/obj/{}.o", basename);

                if (!needsRebuild(file, Obj)) {
                    if (local.debug)
                        hp::printlnCl(std::format("[HelpMake] Config '{}' Skip (up to date): {}", name, file), hp::YELLOW);
                    objs.push_back(Obj);
                    continue;
                }

                std::string objCmd = std::format("{} -fdiagnostics-color=always {} -c \"{}\" -o \"{}\" -MMD", comp, CommandFlags, file, Obj);

                if (cfg.debug)
                    hp::printlnCl(std::format("[HelpMake] Config '{}': Object Command: {}\n", name, objCmd), hp::Color::YELLOW);

                std::string redirectCmd = std::format("{} > \"{}\" 2>&1", objCmd, r_logPath);
                exitCode = std::system(redirectCmd.c_str());
                objs.push_back(Obj);
            }

            if (exitCode == 0) {

                std::string linkCmd = comp + " -fdiagnostics-color=always " + CommandFlags;
                for (const auto &obj : objs)
                    linkCmd += " \"" + obj + "\"";
                linkCmd += " -o \"" + local.output + "\"" + (local.run ? " && " + local.output : "");

                if (cfg.debug)
                    hp::printlnCl(std::format("\n[HelpMake] Config '{}': Link Command: {}", name, linkCmd), hp::Color::YELLOW);

                std::string redirectCmd = std::format("{} > \"{}\" 2>&1", linkCmd, r_logPath);
                exitCode = std::system(redirectCmd.c_str());
            } else {
                hp::printlnCl(std::format("Config '{}' compilation failed.", name), hp::Color::RED);
                exit(EXIT_FAILURE);
            }

        } else {
            command = std::format("{} -fdiagnostics-color=always {} {} -o {} {}",
                                  comp, CommandFlags, local.inputFile, local.output, (local.run ? "&& " + local.output : ""));

            if (cfg.debug)
                hp::printlnCl(std::format("[HelpMake] Config '{}': Real Command: {}", name, command), hp::Color::YELLOW);

            std::string redirectCmd = std::format("{} > \"{}\" 2>&1", command, r_logPath);
            exitCode = std::system(redirectCmd.c_str());
        }

        if (exitCode == 0)
            hp::printlnCl(std::format("Config '{}' compiled successfully. Output: {}", name, local.output), hp::Color::GREEN);
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

    if (cfg.verbose)
        hp::printlnCl(std::format("\n[Help_Make] Downloading dependency '{}' ...\n", url), hp::Color::CYAN);

    std::size_t slash = url.find_last_of("/");
    std::string include = url.substr(slash + 1);
    if (include.size() > 4 && include.substr(include.size() - 4) == ".git")
        include.erase(include.size() - 4);

    hp::command(std::format("git clone {} build/HelpMake/dep/{}", url, include));

    std::string fullPath = std::format("build/HelpMake/dep/{}", include);
    if (!folder.empty() && folder != ".")
        fullPath += "/" + folder;

    if (cfg.debug) {
        hp::printlnCl(std::format("[Debug] Fetch Include Folder: '{}'", folder), hp::Color::YELLOW);
        hp::printlnCl(std::format("[Debug] Fetch Clone Folder: '{}'", include), hp::Color::YELLOW);
    }

    return fullPath;
}

void Parser::generateCompileCommands() {
    std::ofstream json("compile_commands.json");
    if (!json.is_open()) {
        if (!cfg.quiet)
            hp::printlnCl("Warning: Could not write compile_commands.json", hp::Color::YELLOW);
        return;
    }

    std::string cwd = std::filesystem::current_path().generic_string();
    std::string comp;
    comp = compilerCommand(cfg.compiler);

    std::string incFlags;
    for (const auto &inc : cfg.includeFiles)
        incFlags += std::format(" -I{}", inc);

    json << "[\n";
    bool first = true;
    for (const auto &file : cfg.v_inputFiles) {
        if (!first)
            json << ",\n";
        first = false;

        std::string command = std::format("{} -{} {} -o {} {}{}",
                                          comp, cfg.version, file, cfg.output, cfg.flags, incFlags);

        json << "  {\n";
        json << "    \"directory\": \"" << cwd << "\",\n";
        json << "    \"command\": \"" << command << "\",\n";
        json << "    \"file\": \"" << file << "\"\n";
        json << "  }";
    }
    json << "\n]\n";
    json.close();

    if (cfg.verbose)
        hp::printlnCl("Generated compile_commands.json", hp::Color::GREEN);
}

bool Parser::needsRebuild(const std::string &src, const std::string &obj) {
    if (cfg.rebuild)
        return true;
    if (!std::filesystem::exists(obj))
        return true;

    auto srcTime = std::filesystem::last_write_time(src);
    auto objTime = std::filesystem::last_write_time(obj);

    std::ifstream depFile(std::filesystem::path(obj).replace_filename(std::filesystem::path(obj).stem().string() + ".d").generic_string());
    if (!depFile.is_open()) {
        if (!cfg.quiet)
            hp::printlnCl(std::format("Warning: Could not open dependency file for '{}'. Rebuilding.", obj), hp::Color::YELLOW);
        return true;
    }
    std::string line;
    auto depTime = std::filesystem::file_time_type::min();

    if (srcTime > objTime) {
        if (cfg.debug)
            hp::printlnCl(std::format("[Debug] Source file '{}' is newer than object '{}'. Rebuilding.", src, obj), hp::Color::YELLOW);
        return true;
    }

    while (std::getline(depFile, line)) {
        std::istringstream iss(line);
        std::string depFilePath;
        while (iss >> depFilePath) {
            if (depFilePath.find(":") != std::string::npos || depFilePath == "\\" || (depFilePath.length() <= 2 && depFilePath.back() == '\\'))
                continue;

            if (!std::filesystem::exists(depFilePath)) {
                if (!cfg.quiet)
                    hp::printlnCl(std::format("Warning: Dependency file '{}' does not exist. Rebuilding.", depFilePath), hp::Color::YELLOW);
                return true;
            }

            depTime = std::filesystem::last_write_time(depFilePath);
            if (depTime > objTime) {
                if (cfg.debug)
                    hp::printlnCl(std::format("[Debug] Dependency '{}' is newer than object '{}'. Rebuilding.", depFilePath, obj), hp::Color::YELLOW);
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> Parser::splitArgs(const std::string &args) {
    std::vector<std::string> result;
    std::istringstream iss(args);
    std::string token;
    while (iss >> token)
        result.push_back(token);
    return result;
}

std::vector<std::string> Parser::getInputFile() {
    return cfg.v_inputFiles;
}
std::vector<std::string> Parser::getInclude() {
    return cfg.includeFiles;
}
std::vector<std::string> Parser::getFlags() {
    return cfg.v_Flags;
}
std::vector<std::string> Parser::getModules() {
    return cfg.v_Modules;
}
std::vector<std::string> Parser::getGithub() {
    return cfg.v_Github;
}
std::string Parser::getCompiler() {
    return cfg.compiler;
}
std::string Parser::getOutput() {
    return cfg.output;
}
std::string Parser::getVersion() {
    return cfg.version;
}