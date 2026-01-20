#include "interpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

#define ASM_LINE(x) asmStream << x << std::endl;

void Interpreter::toASM(char* code, const char* filename, bool optimize) {
    /* std::ofstream asmFile(filename);
    if (!asmFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    } */
    // dont write to file yet, just use a string then write to file at the end
    std::stringstream asmStream;

    std::stack<int> loopStack;

    ASM_LINE("bits 64")
    ASM_LINE("section .data")
    #ifdef __linux__
    //ASM_LINE("carriage_ret db 10")
    ASM_LINE("section .bss")
    ASM_LINE("tape resb 30000")
    ASM_LINE("section .text")
    ASM_LINE()
    ASM_LINE("global _start")
    ASM_LINE("_start:")
    ASM_LINE("    mov rbx, tape")
    #else
    //ASM_LINE("carriage_ret db 13")
    ASM_LINE("cr db 13")
    ASM_LINE("section .bss")
    ASM_LINE("tape resb 30000")
    ASM_LINE("section .text")
    ASM_LINE("global main")
    ASM_LINE("extern GetStdHandle")
    ASM_LINE("extern WriteFile")
    ASM_LINE("extern ExitProcess")
    ASM_LINE("extern ReadFile")
    ASM_LINE("main:")
    ASM_LINE("    sub rsp, 40")
    ASM_LINE("    mov rbx, tape")
    #endif
    const char* READ_COMMAND =
    #ifdef __linux__
        "    mov rax, 0\n"
        "    mov rdi, 0\n"
        "    lea rsi, [rbx]\n"
        "    mov rdx, 1\n"
        "    syscall\n";
    #else
        "    mov ecx, -10\n" 
        "    call GetStdHandle\n"
        "    mov rcx, rax\n" 
        "    lea rdx, [rbx]\n"
        "    mov r8d, 1\n"
        "    lea r9, [rsp+32]\n"
        "    mov qword [rsp+32], 0\n"
        "    call ReadFile\n";
    #endif


    for (int i = 0; code[i] != 0; ++i) {
        char cur = code[i];
        switch (cur) {
            case '>':
                ASM_LINE("    inc rbx")
                break;
            case '<':
                ASM_LINE("    dec rbx")
                break;
            case '+':
                ASM_LINE("    inc byte [rbx]")
                break;
            case '-':
                ASM_LINE("    dec byte [rbx]")
                break;
            case '.':
                #ifdef __linux__
                    ASM_LINE("    mov rax, 1")
                    ASM_LINE("    mov rdi, 1")
                    ASM_LINE("    lea rsi, [rbx]")
                    ASM_LINE("    mov rdx, 1")
                    ASM_LINE("    syscall")
                #else
                    ASM_LINE("    mov ecx, -11")
                    ASM_LINE("    call GetStdHandle")
                    ASM_LINE("    mov rcx, rax")
                    ASM_LINE("    lea rdx, [rbx]")
                    ASM_LINE("    mov r8d, 1")
                    ASM_LINE("    lea r9, [rsp+32]")
                    ASM_LINE("    mov qword [rsp+32], 0")
                    ASM_LINE("    call WriteFile")
                #endif
                break;

            case ',':
                ASM_LINE(READ_COMMAND)
                break;
            case '[':
                loopStack.push(i);
                ASM_LINE("loop_start_" << i << ":")
                ASM_LINE("    cmp byte [rbx], 0")
                ASM_LINE("    je loop_end_" << i)
                break;
            case ']':
                if (loopStack.empty()) {
                    std::cerr << "Error: unmatched ']' at position " << i << std::endl;
                    return;
                }
                {
                    int start = loopStack.top();
                    loopStack.pop();
                    ASM_LINE("    cmp byte [rbx], 0")
                    ASM_LINE("    jne loop_start_" << start)
                    ASM_LINE("loop_end_" << start << ":")
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

    #ifdef __linux__
    ASM_LINE("    mov rax, 60")
    ASM_LINE("    xor rdi, rdi")
    ASM_LINE("    syscall")
    #else
    ASM_LINE("    mov ecx, 0")
    ASM_LINE("    call ExitProcess")
    #endif

    std::string asmCode = asmStream.str();
    if (optimize) {
        optimizeASM(asmCode);
    }

    std::ofstream asmFile(filename);
    if (!asmFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }
    asmFile << asmCode;

    asmFile.close();

    std::cout << "Assembly code written to " << filename << std::endl;
    std::cout << "You can assemble and link it using NASM and a linker like gcc via msys2" << std::endl;
}

void Interpreter::optimizeASM(std::string& asmCode) {
    // anais gumball brainfart gif

    // okay so this doesn't work yet idk
    // im tyring to make it so it doesn't have repeated instructions over and over
    // but it doesn't allow character outputs??? idk i dont got time to look at it right now

    // i also need to charge my laptop lmao
    std::istringstream in(asmCode);
    std::ostringstream out;

    std::cout << "Optimizing assembly code..." << std::endl;

    std::string line;
    int cellDelta = 0;
    int ptrDelta = 0;
    bool handleInitialized = false;

    auto flush = [&]() {
        if (cellDelta != 0) {
            out << "    " << (cellDelta > 0 ? "add" : "sub")
                << " byte [rbx], " << abs(cellDelta) << "\n";
            cellDelta = 0;
        }
        if (ptrDelta != 0) {
            out << "    " << (ptrDelta > 0 ? "add" : "sub")
                << " rbx, " << abs(ptrDelta) << "\n";
            ptrDelta = 0;
        }
    };

    #ifndef __linux__
        out << "PRINT_CHAR:\n"
            << "    push rbx\n"
            << "    sub rsp, 8\n"
            << "    mov rcx, r12\n"
            << "    lea rdx, [rbx]\n"
            << "    mov r8d, 1\n"
            << "    lea r9, [rsp]\n"
            << "    mov qword [rsp], 0\n"
            << "    call WriteFile\n"
            << "    add rsp, 8\n"
            << "    pop rbx\n"
            << "    ret\n\n";
    #endif

    while (std::getline(in, line)) {
        if (line.find(':') != std::string::npos ||
            line.find("jmp") != std::string::npos ||
            line.find("je") != std::string::npos ||
            line.find("jne") != std::string::npos ||
            line.find("syscall") != std::string::npos ||
            line.find("call ExitProcess") != std::string::npos ||
            line.find("mov rax, 60") != std::string::npos ||
            line.empty()) {
            flush();
            out << line << "\n";
            continue;
        }

        #ifndef __linux__
            if (!handleInitialized && line.find("mov rbx, tape") != std::string::npos) {
                flush();
                out << line << "\n"
                    << "    mov ecx, -11\n"
                    << "    call GetStdHandle\n"
                    << "    mov r12, rax\n";
                handleInitialized = true;
                continue;
            }

            if (line.find("mov ecx, -11") != std::string::npos ||
                line.find("call GetStdHandle") != std::string::npos ||
                line.find("lea rdx, [rbx]") != std::string::npos ||
                line.find("call WriteFile") != std::string::npos) {
                flush();
                out << "    call PRINT_CHAR\n";
                continue;
            }
        #endif

        if (line.find("inc byte [rbx]") != std::string::npos) {
            cellDelta++;
        } else if (line.find("dec byte [rbx]") != std::string::npos) {
            cellDelta--;
        } else if (line.find("inc rbx") != std::string::npos) {
            ptrDelta++;
        } else if (line.find("dec rbx") != std::string::npos) {
            ptrDelta--;
        } else {
            flush();
            out << line << "\n";
        }
    }

    flush();

    asmCode = out.str();
    std::cout << "Optimization complete." << std::endl;
}
