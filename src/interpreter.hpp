#pragma once
#include <string>

class Interpreter {
public:
    void toASM(char* code, const char* filename, bool optimize = false);
    void optimizeASM(std::string& asmCode);
private:
    static const int TAPE_SIZE = 30000; // Standard Brainfuck tape size
    unsigned char tape[TAPE_SIZE] = {0};
    unsigned char* pointer = tape;
};