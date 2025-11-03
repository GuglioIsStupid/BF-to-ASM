#include "interpreter.hpp"
#include <iostream>
#include <fstream>
#include <stack>

void Interpreter::toASM(char* code, const char* filename) {
    std::ofstream asmFile(filename);
    if (!asmFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    std::stack<int> loopStack;

    asmFile << "section .data\n";
    asmFile << "carriage_ret db 13\n";
    asmFile << "section .bss\n";
    asmFile << "tape resb 30000\n";
    asmFile << "section .text\n";
    asmFile << "global main\n";
    asmFile << "extern GetStdHandle\n";
    asmFile << "extern WriteFile\n";
    asmFile << "extern ExitProcess\n\n";
    asmFile << "main:\n";
    asmFile << "    sub rsp, 40\n";
    asmFile << "    mov rbx, tape\n";

    for (int i = 0; code[i] != 0; ++i) {
        char cur = code[i];
        switch (cur) {
            case '>':
                asmFile << "    inc rbx\n";
                break;
            case '<':
                asmFile << "    dec rbx\n";
                break;
            case '+':
                asmFile << "    inc byte [rbx]\n";
                break;
            case '-':
                asmFile << "    dec byte [rbx]\n";
                break;
            case '.':
                asmFile << "    mov al, [rbx]\n";
                asmFile << "    cmp al, 10\n";
                asmFile << "    jne not_newline_" << i << "\n";
                asmFile << "    mov ecx, -11\n";
                asmFile << "    call GetStdHandle\n";
                asmFile << "    mov rcx, rax\n";
                asmFile << "    lea rdx, [rel carriage_ret]\n";
                asmFile << "    mov r8d, 1\n";
                asmFile << "    lea r9, [rsp+32]\n";
                asmFile << "    mov qword [rsp+32], 0\n";
                asmFile << "    call WriteFile\n";
                asmFile << "not_newline_" << i << ":\n";

                asmFile << "    mov ecx, -11\n";
                asmFile << "    call GetStdHandle\n";
                asmFile << "    mov rcx, rax\n";
                asmFile << "    lea rdx, [rbx]\n";
                asmFile << "    mov r8d, 1\n";
                asmFile << "    lea r9, [rsp+32]\n";
                asmFile << "    mov qword [rsp+32], 0\n";
                asmFile << "    call WriteFile\n";
                break;
            case '[':
                loopStack.push(i);
                asmFile << "loop_start_" << i << ":\n";
                asmFile << "    cmp byte [rbx], 0\n";
                asmFile << "    je loop_end_" << i << "\n";
                break;
            case ']':
                if (loopStack.empty()) {
                    std::cerr << "Error: unmatched ']' at position " << i << std::endl;
                    return;
                }
                {
                    int start = loopStack.top();
                    loopStack.pop();
                    asmFile << "    cmp byte [rbx], 0\n";
                    asmFile << "    jne loop_start_" << start << "\n";
                    asmFile << "loop_end_" << start << ":\n";
                }
                break;
            default:
                break;
        }
    }

    if (!loopStack.empty()) {
        std::cerr << "Error: unmatched '[' in code." << std::endl;
        return;
    }

    asmFile << "    xor ecx, ecx\n";
    asmFile << "    call ExitProcess\n";

    asmFile.close();

    std::cout << "Assembly code written to " << filename << std::endl;
    std::cout << "You can assemble and link it using NASM and a linker like gcc via msys2" << std::endl;
}
