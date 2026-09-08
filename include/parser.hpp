#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

class Parser {
  private:
    std::filesystem::path filename;
    std::string compiler;
    std::string version;
    std::string output;
    std::string flags;
    std::string inputFile;

    bool isInputFileSet = false;
    bool isIncludeSet = false;
    bool isGithubSet = false;
    bool verbose = false;
    bool debug = false;
    bool run = false;
    bool n_file = false;

    std::vector<std::string> includeFiles;

  public:
    Parser(std::filesystem::path file, std::string inputFile, std::string comp, std::string ver,
           std::string out, std::string flg, bool verbose, bool run, bool debug, bool n_file)
        : filename(file), inputFile(inputFile), compiler(comp), version(ver), output(out), flags(flg),
          verbose(verbose), run(run), debug(debug), n_file(n_file) {}

    void parse();
    void execute();
    void buildCommand();
};