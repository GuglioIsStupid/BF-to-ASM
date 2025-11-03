#include <iostream>
#include <fstream>
#include <string>

#include "interpreter.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string nameWithoutExt = std::string(argv[1]);
    size_t lastindex = nameWithoutExt.find_last_of(".");
    if (lastindex != std::string::npos) {
        nameWithoutExt = nameWithoutExt.substr(0, lastindex);
    }
    std::string outputFilename = nameWithoutExt + ".asm";

    Interpreter interpreter;
    interpreter.toASM(const_cast<char*>(code.c_str()), outputFilename.c_str());

    return 0;
}
