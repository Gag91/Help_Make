#include "HelpMake.hpp"

int main() {
    hp::HelpMake helpMake;

    helpMake.setCompiler("gcc");
    helpMake.setVersion("std=c++26");
    helpMake.setIncludeFiles({"include", "src"});
    helpMake.setOutput("out.exe");
    helpMake.setVerbose(true);
    helpMake.setDebug(true);

    helpMake.build();
}