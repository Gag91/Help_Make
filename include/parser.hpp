#pragma once
#include <filesystem>
#include <fstream>
#include <string>

class Parser {
  private:
    std::filesystem::path filename;
    std::string compiler;
    std::string version;
    std::string output;
    std::string flags;
    std::string inputFile;
    bool isInputFileSet = false;
    bool verbose = false;
    bool run = false;

  public:
    Parser(std::filesystem::path file, std::string inputFile, std::string comp, std::string ver, std::string out, std::string flg, bool verbose, bool run) : filename(file), inputFile(inputFile), compiler(comp), version(ver), output(out), flags(flg), verbose(verbose), run(run) {}

    void parse();
    void execute();
};