#include "../header/interpreter.hpp"
#include <iostream>
#include <algorithm>

Interpreter::Interpreter() : ip(0), bp(0) {}

void Interpreter::ensureStackSpace(int needed) {
    if (stack.size() + needed > MAX_STACK_SIZE) {
        throw RuntimeError("stack overflow: max stack size " +
                           std::to_string(MAX_STACK_SIZE) + " exceeded");
    }
}

int Interpreter::resolveAddress(int level, int addr) {
    int frame = bp;
    for (int i = 0; i < level; ++i) {
        if (frame < 0 || frame >= static_cast<int>(stack.size())) {
            throw RuntimeError("stack corruption: invalid frame pointer");
        }
        frame = stack[frame];
    }
    int target = frame + addr;
    if (target < 0 || target >= static_cast<int>(stack.size())) {
        throw RuntimeError("out-of-bounds stack access: address " +
                           std::to_string(addr) + " in frame " +
                           std::to_string(frame) + " resolves to index " +
                           std::to_string(target));
    }
    return target;
}

void Interpreter::handleInt(int level, int operand) {
    ensureStackSpace(operand);
    int frameStart = static_cast<int>(stack.size());

    int sl = 0;
    if (level > 0) {
        int parent = bp;
        for (int i = 1; i < level; ++i) {
            if (parent < 0 || parent >= static_cast<int>(stack.size())) {
                throw RuntimeError("stack corruption in INT: cannot resolve static link");
            }
            parent = stack[parent];
        }
        sl = parent;
    }

    stack.push_back(sl);
    stack.push_back(bp);
    stack.push_back(0);

    for (int i = 3; i < operand; ++i) {
        stack.push_back(0);
    }

    bp = frameStart;
}

void Interpreter::handleLit(int operand) {
    ensureStackSpace(1);
    stack.push_back(operand);
}

void Interpreter::handleLod(int level, int address) {
    ensureStackSpace(1);
    int target = resolveAddress(level, address);
    stack.push_back(stack[target]);
}

void Interpreter::handleSto(int level, int address) {
    if (stack.empty()) {
        throw RuntimeError("stack underflow in STO: empty stack");
    }
    int value = stack.back();
    stack.pop_back();
    int target = resolveAddress(level, address);
    stack[target] = value;
}

void Interpreter::handleCal(int operand) {
    ensureStackSpace(3);
    int sl = 0;
    if (operand > 0) {
        int parent = bp;
        for (int i = 1; i < operand; ++i) {
            if (parent < 0 || parent >= static_cast<int>(stack.size())) {
                throw RuntimeError("stack corruption in CAL: invalid frame chain");
            }
            parent = stack[parent];
        }
        sl = parent;
    }

    stack.push_back(sl);
    stack.push_back(bp);
    stack.push_back(ip + 1);

    bp = static_cast<int>(stack.size()) - 3;

    if (operand < 0 || operand >= static_cast<int>(code.size())) {
        throw RuntimeError("invalid jump target in CAL: line " +
                           std::to_string(operand) + " is out of range");
    }
    ip = operand;
}

void Interpreter::handleJmp(int operand) {
    if (operand < 0 || operand >= static_cast<int>(code.size())) {
        throw RuntimeError("invalid jump target in JMP: line " +
                           std::to_string(operand) + " is out of range");
    }
    ip = operand;
}

void Interpreter::handleJpc(int operand) {
    if (stack.empty()) {
        throw RuntimeError("stack underflow in JPC: empty stack");
    }
    int condition = stack.back();
    stack.pop_back();

    if (!condition) {
        if (operand < 0 || operand >= static_cast<int>(code.size())) {
            throw RuntimeError("invalid jump target in JPC: line " +
                               std::to_string(operand) + " is out of range");
        }
        ip = operand;
    }
}

void Interpreter::handleOpr(int operand) {
    OprCode op = static_cast<OprCode>(operand);
    int rhs, lhs;

    switch (op) {
        case OprCode::NEG:
            if (stack.empty()) throw RuntimeError("stack underflow in NEG");
            stack.back() = -stack.back();
            break;

        case OprCode::ADD:
            if (stack.size() < 2) throw RuntimeError("stack underflow in ADD");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs + rhs);
            break;

        case OprCode::SUB:
            if (stack.size() < 2) throw RuntimeError("stack underflow in SUB");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs - rhs);
            break;

        case OprCode::MUL:
            if (stack.size() < 2) throw RuntimeError("stack underflow in MUL");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs * rhs);
            break;

        case OprCode::DIV:
            if (stack.size() < 2) throw RuntimeError("stack underflow in DIV");
            rhs = stack.back(); stack.pop_back();
            if (rhs == 0) throw RuntimeError("division by zero");
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs / rhs);
            break;

        case OprCode::MOD:
            if (stack.size() < 2) throw RuntimeError("stack underflow in MOD");
            rhs = stack.back(); stack.pop_back();
            if (rhs == 0) throw RuntimeError("modulo by zero");
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs % rhs);
            break;

        case OprCode::EQL:
            if (stack.size() < 2) throw RuntimeError("stack underflow in EQL");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs == rhs ? 1 : 0);
            break;

        case OprCode::NEQ:
            if (stack.size() < 2) throw RuntimeError("stack underflow in NEQ");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs != rhs ? 1 : 0);
            break;

        case OprCode::LSS:
            if (stack.size() < 2) throw RuntimeError("stack underflow in LSS");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs < rhs ? 1 : 0);
            break;

        case OprCode::GEQ:
            if (stack.size() < 2) throw RuntimeError("stack underflow in GEQ");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs >= rhs ? 1 : 0);
            break;

        case OprCode::GTR:
            if (stack.size() < 2) throw RuntimeError("stack underflow in GTR");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs > rhs ? 1 : 0);
            break;

        case OprCode::LEQ:
            if (stack.size() < 2) throw RuntimeError("stack underflow in LEQ");
            rhs = stack.back(); stack.pop_back();
            lhs = stack.back(); stack.pop_back();
            stack.push_back(lhs <= rhs ? 1 : 0);
            break;

        case OprCode::WRT:
            if (stack.empty()) throw RuntimeError("stack underflow in WRT");
            std::cout << stack.back();
            stack.pop_back();
            break;

        case OprCode::WRTLN:
            if (stack.empty()) throw RuntimeError("stack underflow in WRTLN");
            std::cout << stack.back() << std::endl;
            stack.pop_back();
            break;

        default:
            throw RuntimeError("unknown OPR code: " + std::to_string(operand));
    }
}

void Interpreter::handleRet() {
    if (bp < 0 || bp + 2 >= static_cast<int>(stack.size())) {
        throw RuntimeError("stack corruption in RET: invalid frame");
    }
    int returnAddr = stack[bp + 2];
    int oldBp = stack[bp + 1];

    if (returnAddr == 0) {
        ip = static_cast<int>(code.size());
        return;
    }

    stack.resize(bp);
    bp = oldBp;
    ip = returnAddr;
}

void Interpreter::execute(const std::vector<Instruction>& code) {
    this->code = code;
    ip = 0;
    bp = 0;
    stack.clear();

    while (ip >= 0 && ip < static_cast<int>(code.size())) {
        const Instruction& inst = code[ip];
        int currentIp = ip;
        ++ip;

        switch (inst.op) {
            case OpCode::INT: handleInt(inst.level, inst.operand); break;
            case OpCode::LIT: handleLit(inst.operand); break;
            case OpCode::LOD: handleLod(inst.level, inst.operand); break;
            case OpCode::STO: handleSto(inst.level, inst.operand); break;
            case OpCode::CAL: handleCal(inst.operand); break;
            case OpCode::JMP: handleJmp(inst.operand); break;
            case OpCode::JPC: handleJpc(inst.operand); break;
            case OpCode::OPR: handleOpr(inst.operand); break;
            case OpCode::RET: handleRet();

                if (ip >= static_cast<int>(code.size())) return;
                break;
            default:
                throw RuntimeError("unknown opcode at line " +
                                   std::to_string(currentIp));
        }
    }
}
