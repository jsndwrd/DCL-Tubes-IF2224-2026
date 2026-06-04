#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

enum class OpCode {
    INT,  // init memory frame
    LIT,  // load literal value
    LOD,  // load variable value
    STO,  // store to variable
    CAL,  // call prod/func
    JMP,  // unconditional jump
    JPC,  // conditional jump (if false)
    OPR,  // execute op
    RET   // return from prod/func
};

enum class OprCode {
    NEG = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    MOD = 6,
    EQL = 7,
    NEQ = 8,
    LSS = 9,
    GEQ = 10,
    GTR = 11,
    LEQ = 12,
    WRT = 13,
    WRTLN = 14
};

struct Instruction {
    int line;
    OpCode op;
    int level;
    int operand;
};

inline const char* opCodeName(OpCode op) {
    switch (op) {
        case OpCode::INT: return "INT";
        case OpCode::LIT: return "LIT";
        case OpCode::LOD: return "LOD";
        case OpCode::STO: return "STO";
        case OpCode::CAL: return "CAL";
        case OpCode::JMP: return "JMP";
        case OpCode::JPC: return "JPC";
        case OpCode::OPR: return "OPR";
        case OpCode::RET: return "RET";
        default:          return "???";
    }
}

inline void printInstructions(const std::vector<Instruction>& code,
                               std::ostream& os = std::cout) {
    for (const auto& i : code) {
        os << std::setw(4) << i.line << " "
           << std::left << std::setw(4) << opCodeName(i.op)
           << std::right
           << " " << std::setw(2) << i.level
           << " " << std::setw(4) << i.operand
           << "\n";
    }
}

#endif
