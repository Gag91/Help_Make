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
    std::string modules;
    std::string Precmd;
    std::string Postcmd;

    bool isInputFileSet = false;
    bool isIncludeSet = false;
    bool isGithubSet = false;
    bool isFlagsSet = false;
    bool isModulesSet = false;
    bool isPreBuildSet = false;
    bool isPostBuildSet = false;
    bool verbose = false;
    bool debug = false;
    bool run = false;
    bool n_file = false;
    bool buildSere = false;

    std::vector<std::string> includeFiles;
    std::vector<std::string> v_inputFiles;
    std::vector<std::string> v_Flags;
    std::vector<std::string> v_Modules;
    std::vector<std::string> v_Github;

  public:
    Parser(std::filesystem::path file, std::string inputFile, std::string comp, std::string ver,
           std::string out, std::string flg, bool verbose, bool run, bool debug, bool n_file, bool buildSere)
        : filename(file), inputFile(inputFile), compiler(comp), version(ver), output(out), flags(flg),
          verbose(verbose), run(run), debug(debug), n_file(n_file), buildSere(buildSere) {}

    void parse();
    void execute();
    void buildCommand();
    std::vector<std::string> getInputFile();
    std::vector<std::string> getInclude();
    std::vector<std::string> getFlags();
    std::vector<std::string> getModules();
    std::vector<std::string> getGithub();
};