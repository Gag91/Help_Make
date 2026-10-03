#pragma once

#include "parser.hpp"

#include <concepts>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace hp {
    class HelpMake;

    template <typename T>
    concept StringLike = std::convertible_to<T, std::string>;

    class HelpMake {
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
        bool isFetchSet = false;
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

        std::vector<std::string> includeFiles;
        std::vector<std::string> v_inputFiles;
        std::vector<std::string> v_Flags;
        std::vector<std::string> v_Modules;
        std::vector<std::string> v_Github;
        std::vector<std::string> v_Postcmd;
        std::vector<std::string> v_Precmd;

        static std::string joinVec(const std::vector<std::string> &v) {
            std::string result;
            for (const auto &s : v) {
                if (!result.empty())
                    result += " ";
                result += s;
            }
            return result;
        }

      public:
        HelpMake() = default;

        template <StringLike T>
        HelpMake &setCompiler(T &&v) {
            compiler = std::string(std::forward<T>(v));
            return *this;
        }

        template <StringLike T>
        HelpMake &setVersion(T &&v) {
            std::string s = std::string(std::forward<T>(v));
            if (s.rfind("std=", 0) != 0)
                s = "std=" + s;
            version = s;
            return *this;
        }

        template <StringLike T>
        HelpMake &setStandard(T &&v) {
            return setVersion(std::forward<T>(v));
        }

        template <StringLike T>
        HelpMake &setOutput(T &&v) {
            output = std::string(std::forward<T>(v));
            return *this;
        }

        template <StringLike T>
        HelpMake &setFlags(T &&v) {
            flags = std::string(std::forward<T>(v));
            isFlagsSet = true;
            return *this;
        }

        template <StringLike T>
        HelpMake &setInputFile(T &&v) {
            inputFile = std::string(std::forward<T>(v));
            isInputFileSet = true;
            return *this;
        }

        template <StringLike T>
        HelpMake &setModules(T &&v) {
            modules = std::string(std::forward<T>(v));
            isModulesSet = true;
            return *this;
        }

        template <StringLike T>
        HelpMake &setPrecmd(T &&v) {
            Precmd = std::string(std::forward<T>(v));
            isPreBuildSet = true;
            return *this;
        }

        template <StringLike T>
        HelpMake &setPostcmd(T &&v) {
            Postcmd = std::string(std::forward<T>(v));
            isPostBuildSet = true;
            return *this;
        }

        template <typename T>
            requires std::convertible_to<T, std::filesystem::path>
        HelpMake &setConfigFile(T &&v) {
            filename = std::filesystem::path(std::forward<T>(v));
            return *this;
        }

        HelpMake &addInputFile(const std::string &f) {
            v_inputFiles.push_back(f);
            isInputFileSet = true;
            return *this;
        }

        HelpMake &addFlag(const std::string &f) {
            v_Flags.push_back(f);
            isFlagsSet = true;
            return *this;
        }

        HelpMake &addInclude(const std::string &i) {
            includeFiles.push_back(i);
            isIncludeSet = true;
            return *this;
        }

        HelpMake &addModule(const std::string &m) {
            v_Modules.push_back(m);
            isModulesSet = true;
            return *this;
        }

        HelpMake &addGithub(const std::string &repo) {
            v_Github.push_back(repo);
            isFetchSet = true;
            return *this;
        }

        HelpMake &addPrecmd(const std::string &cmd) {
            v_Precmd.push_back(cmd);
            isPreBuildSet = true;
            return *this;
        }

        HelpMake &addPostcmd(const std::string &cmd) {
            v_Postcmd.push_back(cmd);
            isPostBuildSet = true;
            return *this;
        }

        HelpMake &setIncludeFiles(std::vector<std::string> include) {
            includeFiles = std::move(include);
            isIncludeSet = true;
            return *this;
        }

        HelpMake &setInputFiles(std::vector<std::string> files) {
            if (!files.empty())
                isInputFileSet = true;
            v_inputFiles = std::move(files);
            return *this;
        }

        HelpMake &setFlagList(std::vector<std::string> flagList) {
            v_Flags = std::move(flagList);
            isFlagsSet = true;
            return *this;
        }

        HelpMake &setModuleList(std::vector<std::string> mods) {
            v_Modules = std::move(mods);
            isModulesSet = true;
            return *this;
        }

        HelpMake &setGithub(std::vector<std::string> repos) {
            v_Github = std::move(repos);
            isFetchSet = true;
            return *this;
        }

        HelpMake &setPostcmd(std::vector<std::string> cmds) {
            v_Postcmd = std::move(cmds);
            isPostBuildSet = true;
            return *this;
        }

        HelpMake &setPrecmd(std::vector<std::string> cmds) {
            v_Precmd = std::move(cmds);
            isPreBuildSet = true;
            return *this;
        }

        HelpMake &setVerbose(bool v = true) {
            verbose = v;
            return *this;
        }

        HelpMake &setDebug(bool v = true) {
            debug = v;
            return *this;
        }

        HelpMake &setRun(bool v = true) {
            run = v;
            return *this;
        }

        HelpMake &setNoFile(bool v = true) {
            n_file = v;
            return *this;
        }

        HelpMake &setBuildSere(bool v = true) {
            buildSere = v;
            return *this;
        }

        HelpMake &setCreate(bool v = true) {
            create = v;
            return *this;
        }

        HelpMake &setAllCfgs(bool v = true) {
            allCfgs = v;
            return *this;
        }

        HelpMake &setConfig(bool v = true) {
            isConfigSet = v;
            return *this;
        }

        std::string getCompiler() const {
            return compiler;
        }
        std::string getVersion() const {
            return version;
        }
        std::string getOutput() const {
            return output;
        }
        std::string getFlags() const {
            return flags;
        }
        std::string getInputFile() const {
            return inputFile;
        }
        const std::vector<std::string> &getInputFiles() const {
            return v_inputFiles;
        }
        const std::vector<std::string> &getFlagList() const {
            return v_Flags;
        }
        const std::vector<std::string> &getIncludeFiles() const {
            return includeFiles;
        }
        const std::vector<std::string> &getModules() const {
            return v_Modules;
        }
        const std::vector<std::string> &getGithub() const {
            return v_Github;
        }

        void build() {
            if (filename.empty())
                filename = "HelpMake.txt";

            std::string joinedInputs = joinVec(v_inputFiles);
            if (!joinedInputs.empty())
                inputFile = joinedInputs;

            std::string joinedFlags = joinVec(v_Flags);
            if (!joinedFlags.empty()) {
                if (!flags.empty()) {
                    flags += " " + joinedFlags;
                } else {
                    flags = joinedFlags;
                }
            }

            std::string joinedModules = joinVec(v_Modules);
            if (!joinedModules.empty()) {
                if (!modules.empty())
                    modules += " " + joinedModules;
                else
                    modules = joinedModules;
            }

            for (const auto &inc : includeFiles)
                flags += " -I" + inc;

            if (!v_Precmd.empty())
                Precmd = joinVec(v_Precmd);

            if (!v_Postcmd.empty())
                Postcmd = joinVec(v_Postcmd);

            Parser parser(filename, inputFile, compiler, version, output, flags,
                          verbose, run, debug, n_file, buildSere, create, allCfgs);
            parser.parse();
            if (!create) {
                parser.executeConfigs({});
                if (!isConfigSet && !allCfgs) {
                    parser.execute();
                }
            }
        }

        void buildAndRun() {
            run = true;
            build();
        }

        void buildConfig(const std::string &name) {
            if (filename.empty())
                filename = "HelpMake.txt";

            Parser parser(filename, inputFile, compiler, version, output, flags,
                          verbose, run, debug, n_file, buildSere, create, allCfgs);
            parser.parse();
            parser.executeConfigs({name});
        }
    };

} // namespace hp