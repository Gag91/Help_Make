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

    std::vector<std::string> preBuild;
    std::vector<std::string> postBuild;
    std::vector<std::string> github;
    std::vector<std::string> includeFiles;
    std::vector<std::string> v_inputFiles;
    std::vector<std::string> v_Flags;

    bool run = false;
};

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
    bool isConfigSet = false;
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

    std::vector<std::string> includeFiles;
    std::vector<std::string> v_inputFiles;
    std::vector<std::string> v_Flags;
    std::vector<std::string> v_Modules;
    std::vector<std::string> v_Github;
    std::vector<std::string> v_Postcmd;
    std::vector<std::string> v_Precmd;
    std::string unused;

    Config currentConfig;
    std::map<std::string, Config> configs;
    int configBraceDepth = 1;

  public:
    Parser(std::filesystem::path file, std::string inputFile, std::string comp, std::string ver,
           std::string out, std::string flg, bool verbose, bool run, bool debug, bool n_file, bool buildSere, bool create, bool allCfgs, bool json, bool seperate, bool rebuild)
        : filename(file), inputFile(inputFile), compiler(comp), version(ver), output(out), flags(flg),
          verbose(verbose), run(run), debug(debug), n_file(n_file), buildSere(buildSere), create(create), allCfgs(allCfgs), json(json), seperate(seperate), rebuild(rebuild) {}

    void parse();
    void execute();
    void buildCommand();
    void executeConfigs(const std::vector<std::string> &names);
    void generateCompileCommands();
    void transformwildcards(const std::string &input);

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