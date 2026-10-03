#pragma once
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

struct Config {
    std::string name;

    std::string compiler;
    std::string version;
    std::string output;
    std::string flags;
    std::string inputFile;
    std::string includes;
    std::string modules;
    std::string Precmd;
    std::string Postcmd;

    std::vector<std::string> preBuild;
    std::vector<std::string> postBuild;

    std::vector<std::string> includeFiles;
    std::vector<std::string> v_inputFiles;
    std::vector<std::string> v_Flags;
    std::vector<std::string> v_Modules;
    std::vector<std::string> v_Github;
    std::vector<std::string> v_Postcmd;
    std::vector<std::string> v_Precmd;

    bool verbose = false;
    bool debug = false;
    bool run = false;
    bool n_file = false;
    bool buildSere = false;
    bool create = false;
    bool allCfgs = false;
    bool json = false;
    bool seperate = true;
    bool rebuild = false;
};

class Parser {
  private:
    std::filesystem::path filename;
    Config cfg;

    bool isInputFileSet = false;
    bool isIncludeSet = false;
    bool isFetchSet = false;
    bool isFlagsSet = false;
    bool isModulesSet = false;
    bool isPreBuildSet = false;
    bool isPostBuildSet = false;
    bool isConfigSet = false;

    Config currentConfig;
    std::map<std::string, Config> configs;
    int configBraceDepth = 1;
    static std::vector<std::string> splitArgs(const std::string &args);

  public:
    Parser(std::filesystem::path file, Config config)
        : filename(file), cfg(config) {}

    void parse();
    void execute();
    void buildCommand();
    void executeConfigs(const std::vector<std::string> &names);
    void generateCompileCommands();

    std::vector<std::string> getInputFile();
    std::vector<std::string> getInclude();
    std::vector<std::string> getFlags();
    std::vector<std::string> getModules();
    std::vector<std::string> getGithub();

    std::string processGithubEntry(const std::string &value);
    std::string getCompiler();
    std::string getOutput();
    std::string getVersion();

    bool needsRebuild(const std::string &src, const std::string &obj);
};