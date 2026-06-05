#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "instruction.hpp"
#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

class Interpreter {
public:
    Interpreter();

    void execute(const std::vector<Instruction>& code);

    static constexpr int MAX_STACK_SIZE = 10000;
    static constexpr int FRAME_HEADER_SIZE = 3;

private:
    std::vector<int> stack;
    int ip;
    int bp;
    std::vector<Instruction> code;

    void handleInt(int level, int operand);
    void handleLit(int operand);
    void handleLod(int level, int address);
    void handleSto(int level, int address);
    void handleCal(int level, int operand);
    void handleJmp(int operand);
    void handleJpc(int operand);
    void handleOpr(int operand);
    void handleRet(int level, int operand);

    int resolveAddress(int level, int address);
    void ensureStackSpace(int needed);
};

#endif
