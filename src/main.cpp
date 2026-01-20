#include <iostream>
#include <fstream>
#include <string>

#include "interpreter.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    bool OPTIMIZE = false;
    std::cout << "BF Compiler v1.0" << std::endl;
    std::cout << "-----------------" << std::endl;
    std::cout << "Input file: " << argv[1] << std::endl;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--optimize") {
            OPTIMIZE = true;
            break;
        }
    }
    std::cout << "Optimization: " << (OPTIMIZE ? "Enabled" : "Disabled") << std::endl;
    std::cout << "-----------------" << std::endl;
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string outputFilename;
    if (OPTIMIZE && argc >= 3) {
        outputFilename = std::string(argv[2]);
    } else if (!OPTIMIZE && argc >= 2) {
        outputFilename = std::string(argv[2]);
    } else {
        std::string nameWithoutExt = std::string(argv[1]);
        size_t lastindex = nameWithoutExt.find_last_of(".");
        if (lastindex != std::string::npos) {
            nameWithoutExt = nameWithoutExt.substr(0, lastindex);
        }
        outputFilename = nameWithoutExt + ".asm";
    }

    Interpreter interpreter;
    interpreter.toASM(const_cast<char*>(code.c_str()), outputFilename.c_str(), OPTIMIZE);

    return 0;
}
