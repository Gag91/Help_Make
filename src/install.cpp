#include "hp/other/other.hpp"
#include "hp/time/time.hpp"
#include "parser.hpp"

void Parser::install() {
    hp::printlnCl("[HelpMake] Installing HelpMake...", hp::Color::CYAN);

#ifdef _WIN32
    const std::string installDir = "C:\\HelpMake";
    const std::string cloneCmd = "git clone https://github.com/Gag91/Help_Make C:\\HelpMake";
    const std::string buildCmd = "cd /d C:\\HelpMake && g++ -std=c++20 src/*.cpp -o hm.exe -Iinclude";
    const std::string pathCmd = "setx PATH \"%PATH%;C:\\HelpMake\"";
#else
    const std::string installDir = "~/.local/share/HelpMake";
    const std::string cloneCmd = "git clone https://github.com/Gag91/Help_Make ~/.local/share/HelpMake";
    const std::string buildCmd = "cd ~/.local/share/HelpMake && g++ -std=c++20 src/*.cpp -o hm -Iinclude";
    const std::string pathCmd = "mkdir -p ~/.local/bin && ln -sf ~/.local/share/HelpMake/hm ~/.local/bin/hm";
#endif

    hp::printlnCl("[HelpMake] Downloading...", hp::Color::CYAN);
    if (std::system(cloneCmd.c_str()) != 0) {
        hp::printlnCl("[HelpMake] Error: Downloading failed.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }

    hp::printlnCl("[HelpMake] Compiling...", hp::Color::CYAN);
    if (std::system(buildCmd.c_str()) != 0) {
        hp::printlnCl("[HelpMake] Error: Compilation failed.", hp::Color::RED);
        exit(EXIT_FAILURE);
    }

    hp::printlnCl("[HelpMake] Adding to PATH...", hp::Color::CYAN);
    if (std::system(pathCmd.c_str()) != 0) {
        hp::printlnCl("[HelpMake] Warning: PATH update failed.", hp::Color::YELLOW);
        hp::printlnCl(std::format("[HelpMake] Add {} to PATH manually.", installDir), hp::Color::YELLOW);
        return;
    }

    hp::printlnCl("[HelpMake] Installed successfully.", hp::Color::GREEN);
    hp::printlnCl("[HelpMake] Restart your terminal to use 'hm'.", hp::Color::CYAN);
    exit(EXIT_SUCCESS);
}