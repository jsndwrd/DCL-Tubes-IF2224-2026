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
        if (sym.tab[idx].obj == OBJ_VARIABLE) {
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
    int vsze = sym.btab.empty() ? 0 : sym.btab[0].vsze;
    emit(OpCode::INT, 0, FRAME_HEADER_SIZE + vsze);

    if (n->block && n->block->nodeType == AST_BLOCK) {
        genBlock(static_cast<BlockNode*>(n->block));
    }

    emit(OpCode::RET, 0, 0);
}

void CodeGenerator::genBlock(BlockNode* n) {
    for (ASTNode* stmt : n->stmts) {
        genStmt(stmt);
    }
}

void CodeGenerator::genStmt(ASTNode* n) {
    if (!n) return;
    switch (n->nodeType) {
        case AST_ASSIGN: genAssign(static_cast<AssignNode*>(n)); break;
        case AST_IF:     genIf(static_cast<IfNode*>(n)); break;
        case AST_WHILE:  genWhile(static_cast<WhileNode*>(n)); break;
        case AST_FOR:    genFor(static_cast<ForNode*>(n)); break;
        case AST_REPEAT: genRepeat(static_cast<RepeatNode*>(n)); break;
        case AST_CALL:   genCall(static_cast<CallNode*>(n)); break;
        case AST_BLOCK:  genBlock(static_cast<BlockNode*>(n)); break;
        default: break;
    }
}

void CodeGenerator::genAssign(AssignNode* n) {
    if (n->target->nodeType != AST_VAR) {
        throw CodeGenError("codegen only supports simple variable assignment");
    }
    auto* target = static_cast<VarNode*>(n->target);
    int idx = target->tabIndex;
    if (idx < 0 || idx >= static_cast<int>(sym.tab.size())) {
        throw CodeGenError("unresolved assignment target");
    }

    genExpr(n->value);

    int lvl = levelDiff(target->lexLevel, sym.tab[idx].lev);
    emit(OpCode::STO, lvl, sym.tab[idx].adr);
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
    if (!n) return;
    switch (n->nodeType) {
        case AST_BINOP:   genBinOp(static_cast<BinOpNode*>(n)); break;
        case AST_UNARYOP: genUnaryOp(static_cast<UnaryOpNode*>(n)); break;
        case AST_VAR:     genVar(static_cast<VarNode*>(n)); break;
        case AST_NUMBER:  genNumber(static_cast<NumberNode*>(n)); break;
        case AST_BOOL:    genBool(static_cast<BoolNode*>(n)); break;
        case AST_CHAR:    genChar(static_cast<CharNode*>(n)); break;
        case AST_CALL:    genCall(static_cast<CallNode*>(n)); break;
        default:
            throw CodeGenError("unsupported expression node in codegen");
    }
}

void CodeGenerator::genBinOp(BinOpNode* n) {
    if (n->op == "and" || n->op == "or") {
        genExpr(n->left);
        genExpr(n->right);
        if (n->op == "and") {
            emit(OpCode::OPR, 0, static_cast<int>(OprCode::MUL));
        } else {
            emit(OpCode::OPR, 0, static_cast<int>(OprCode::ADD));
        }
        return;
    }

    genExpr(n->left);
    genExpr(n->right);
    emit(OpCode::OPR, 0, static_cast<int>(binOpToOpr(n->op)));
}

void CodeGenerator::genUnaryOp(UnaryOpNode* n) {
    if (n->op == "-") {
        genExpr(n->operand);
        emit(OpCode::OPR, 0, static_cast<int>(OprCode::NEG));
    } else if (n->op == "+") {
        genExpr(n->operand);
    } else if (n->op == "not") {
        emit(OpCode::LIT, 0, 1);
        genExpr(n->operand);
        emit(OpCode::OPR, 0, static_cast<int>(OprCode::SUB));
    } else {
        throw CodeGenError("unsupported unary operator: " + n->op);
    }
}

void CodeGenerator::genVar(VarNode* n) {
    int idx = n->tabIndex;
    if (idx < 0 || idx >= static_cast<int>(sym.tab.size())) {
        throw CodeGenError("unresolved variable: " + n->name);
    }
    if (sym.tab[idx].obj == OBJ_CONSTANT) {
        emit(OpCode::LIT, 0, sym.tab[idx].adr);
        return;
    }
    int lvl = levelDiff(n->lexLevel, sym.tab[idx].lev);
    emit(OpCode::LOD, lvl, sym.tab[idx].adr);
}

void CodeGenerator::genNumber(NumberNode* n) {
    if (n->isReal) {
        throw CodeGenError("real literals not supported in stack machine");
    }
    emit(OpCode::LIT, 0, std::stoi(n->value));
}

void CodeGenerator::genBool(BoolNode* n) {
    emit(OpCode::LIT, 0, n->value ? 1 : 0);
}

void CodeGenerator::genChar(CharNode* n) {
    int code = n->value.empty() ? 0 : static_cast<unsigned char>(n->value[0]);
    emit(OpCode::LIT, 0, code);
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
