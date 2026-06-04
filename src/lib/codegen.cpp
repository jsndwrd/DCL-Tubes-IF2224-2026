#include "../header/codegen.hpp"
#include <algorithm>

CodeGenerator::CodeGenerator(SymbolTable& sym) : sym(sym) {}

int CodeGenerator::emit(OpCode op, int level, int operand) {
    int idx = nextLine();
    instr.push_back({idx, op, level, operand});
    return idx;
}

void CodeGenerator::patch(int instrIdx, int operand) {
    if (instrIdx < 0 || instrIdx >= static_cast<int>(instr.size())) {
        throw CodeGenError("invalid instruction index for patching");
    }
    instr[instrIdx].operand = operand;
}

int CodeGenerator::levelDiff(int useLevel, int declLevel) const {
    int d = useLevel - declLevel;
    return d < 0 ? 0 : d;
}

void CodeGenerator::resolveBlockAddresses(int btabIdx) {
    if (btabIdx < 0 || btabIdx >= static_cast<int>(sym.btab.size())) return;

    std::vector<int> chain;
    int p = sym.btab[btabIdx].last;
    while (p != 0) {
        chain.push_back(p);
        p = sym.tab[p].link;
    }
    std::reverse(chain.begin(), chain.end());

    int addr = FRAME_HEADER_SIZE;
    for (int idx : chain) {
        int obj = sym.tab[idx].obj;
        if (obj == OBJ_VARIABLE || obj == OBJ_CONSTANT) {
            sym.tab[idx].adr = addr;
            addr += 1;
        }
    }
}

void CodeGenerator::resolveAddresses() {
    for (int b = 0; b < static_cast<int>(sym.btab.size()); ++b) {
        resolveBlockAddresses(b);
    }
}

std::vector<Instruction> CodeGenerator::generate(ASTNode* root) {
    instr.clear();
    resolveAddresses();
    if (root && root->nodeType == AST_PROGRAM) {
        genProgram(static_cast<ProgramNode*>(root));
    }
    return instr;
}

void CodeGenerator::genProgram(ProgramNode* n) {
    (void)n;
}

void CodeGenerator::genBlock(BlockNode* n) {
    (void)n;
}

void CodeGenerator::genStmt(ASTNode* n) {
    (void)n;
}

void CodeGenerator::genAssign(AssignNode* n) {
    (void)n;
}

void CodeGenerator::genIf(IfNode* n) {
    (void)n;
}

void CodeGenerator::genWhile(WhileNode* n) {
    (void)n;
}

void CodeGenerator::genFor(ForNode* n) {
    (void)n;
}

void CodeGenerator::genRepeat(RepeatNode* n) {
    (void)n;
}

void CodeGenerator::genCall(CallNode* n) {
    (void)n;
}

void CodeGenerator::genExpr(ASTNode* n) {
    (void)n;
}

void CodeGenerator::genBinOp(BinOpNode* n) {
    (void)n;
}

void CodeGenerator::genUnaryOp(UnaryOpNode* n) {
    (void)n;
}

void CodeGenerator::genVar(VarNode* n) {
    (void)n;
}

void CodeGenerator::genNumber(NumberNode* n) {
    (void)n;
}

void CodeGenerator::genBool(BoolNode* n) {
    (void)n;
}

void CodeGenerator::genChar(CharNode* n) {
    (void)n;
}

void CodeGenerator::genSubprogram(ASTNode* n) {
    (void)n;
}

OprCode CodeGenerator::binOpToOpr(const std::string& op) const {
    if (op == "+")   return OprCode::ADD;
    if (op == "-")   return OprCode::SUB;
    if (op == "*")   return OprCode::MUL;
    if (op == "/")   return OprCode::DIV;
    if (op == "div") return OprCode::DIV;
    if (op == "mod") return OprCode::MOD;
    if (op == "==")  return OprCode::EQL;
    if (op == "<>")  return OprCode::NEQ;
    if (op == "<")   return OprCode::LSS;
    if (op == ">=")  return OprCode::GEQ;
    if (op == ">")   return OprCode::GTR;
    if (op == "<=")  return OprCode::LEQ;
    throw CodeGenError("unsupported binary operator: " + op);
}
