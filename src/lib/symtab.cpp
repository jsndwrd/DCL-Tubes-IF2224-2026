// Implementasi symbol table dan semantic visitor
#include "../header/symtab.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

SymbolTable::SymbolTable() : level(0), dx(0) {

    tab.resize(33);
    for (int i = 0; i < 33; ++i) {
        tab[i] = {"", 0, 0, 0, 0, 1, 0, 0};
    }

    tab[1] = {"integer", 0, OBJ_TYPE, T_INTEGER, 0, 1, 0, 0};
    tab[2] = {"real",    1, OBJ_TYPE, T_REAL,    0, 1, 0, 0};
    tab[3] = {"char",    2, OBJ_TYPE, T_CHAR,    0, 1, 0, 0};
    tab[4] = {"boolean", 3, OBJ_TYPE, T_BOOLEAN, 0, 1, 0, 0};
    tab[5] = {"string",  4, OBJ_TYPE, T_STRING,  0, 1, 0, 0};

    tab[6] = {"true",  5, OBJ_CONSTANT, T_BOOLEAN, 0, 1, 0, 1};
    tab[7] = {"false", 6, OBJ_CONSTANT, T_BOOLEAN, 0, 1, 0, 0};

    tab[8] = {"readln",  7, OBJ_PROCEDURE, T_VOID, 0, 1, 0, 0};
    tab[9] = {"writeln", 8, OBJ_PROCEDURE, T_VOID, 0, 1, 0, 0};

    btab.push_back({9, 0, 0, 0});
    display.push_back(0);
}

void SymbolTable::enterBlock(bool isProcOrFunc) {
    ++level;
    btab.push_back({0, 0, 0, 0});
    display.push_back(static_cast<int>(btab.size()) - 1);
    dx = 0;
}

void SymbolTable::exitBlock() {
    --level;
    display.pop_back();
    dx = 0; 
}

int SymbolTable::enter(const std::string& id, int obj, int type, int ref, int nrm, int adr) {
    int blockIdx = display.back();
    int lastInBlock = btab[blockIdx].last;

    int p = lastInBlock;
    while (p != 0) {
        if (tab[p].id == id && tab[p].lev == level) {
            throw SemanticError("Semantic error: identifier '" + id + "' is already declared in this scope");
        }
        p = tab[p].link;
    }

    int idx = static_cast<int>(tab.size());
    tab.push_back({id, lastInBlock, obj, type, ref, nrm, level, adr});
    btab[blockIdx].last = idx;

    if (obj == OBJ_VARIABLE || obj == OBJ_CONSTANT) {
        dx += 1; 
        btab[blockIdx].vsze = dx;
    }

    return idx;
}

int SymbolTable::enterParameter(const std::string& id, int type, int ref, bool isVar) {
    int blockIdx = display.back();
    int lastInBlock = btab[blockIdx].last;

    int p = lastInBlock;
    while (p != 0) {
        if (tab[p].id == id && tab[p].lev == level) {
            throw SemanticError("Semantic error: parameter '" + id + "' is already declared");
        }
        p = tab[p].link;
    }

    int idx = static_cast<int>(tab.size());
    tab.push_back({id, lastInBlock, OBJ_VARIABLE, type, ref, isVar ? 0 : 1, level, 0});
    btab[blockIdx].last = idx;
    btab[blockIdx].lpar = idx;
    btab[blockIdx].psze += 1;

    return idx;
}

int SymbolTable::lookup(const std::string& id) const {
    for (int l = level; l >= 0; --l) {
        int blockIdx = display[l];
        int p = btab[blockIdx].last;
        while (p != 0) {
            if (tab[p].id == id) return p;
            p = tab[p].link;
        }
    }
    return -1;
}

int SymbolTable::lookupInCurrentBlock(const std::string& id) const {
    int blockIdx = display.back();
    int p = btab[blockIdx].last;
    while (p != 0) {
        if (tab[p].id == id && tab[p].lev == level) return p;
        p = tab[p].link;
    }
    return -1;
}

int SymbolTable::findType(const std::string& name) const {
    for (int i = 1; i < static_cast<int>(tab.size()); ++i) {
        if (tab[i].obj == OBJ_TYPE && tab[i].id == name) {
            return tab[i].type;
        }
    }
    return T_ERROR;
}

bool SymbolTable::isSimpleType(int type) const {
    return type == T_INTEGER || type == T_REAL || type == T_CHAR || type == T_BOOLEAN || type == T_STRING || type == T_SUBRANGE || type == T_ENUM;
}

bool SymbolTable::isNumericType(int type) const {
    return type == T_INTEGER || type == T_REAL || type == T_SUBRANGE;
}

bool SymbolTable::compatible(int t1, int t2) const {
    if (t1 == t2) return true;
    if ((t1 == T_INTEGER && t2 == T_SUBRANGE) || (t1 == T_SUBRANGE && t2 == T_INTEGER)) return true;
    if (t1 == T_SUBRANGE && t2 == T_SUBRANGE) return true; 
    return false;
}

bool SymbolTable::assignmentCompatible(int targetType, int targetRef, int exprType, int exprRef) const {
    if (targetType == exprType) {
        if (targetType == T_RECORD || targetType == T_ARRAY) {

            return targetRef == exprRef;
        }
        return true;
    }
    if (targetType == T_REAL && exprType == T_INTEGER) return true;
    if (targetType == T_INTEGER && exprType == T_SUBRANGE) return true;
    if (targetType == T_SUBRANGE && exprType == T_INTEGER) return true;
    if (targetType == T_SUBRANGE && exprType == T_SUBRANGE) return true;

    if (targetType == T_SUBRANGE && targetRef >= 0 && targetRef < static_cast<int>(atab.size())) {
        int baseType = atab[targetRef].xtyp;
        if (exprType == baseType) return true;
    }
    return false;
}

void SymbolTable::printTab(std::ostream& os) const {
    os << "\n=== tab ===\n";
    os << std::setw(4) << "idx" << " "
       << std::setw(12) << "id" << " "
       << std::setw(10) << "obj" << " "
       << std::setw(6) << "type" << " "
       << std::setw(5) << "ref" << " "
       << std::setw(4) << "nrm" << " "
       << std::setw(4) << "lev" << " "
       << std::setw(4) << "adr" << " "
       << std::setw(5) << "link" << "\n";
    os << "-----------------------------------------------------------\n";
    for (size_t i = 0; i < tab.size(); ++i) {
        const auto& e = tab[i];
        if (e.id.empty() && i >= 10 && i < 33) continue; 
        std::string objStr;
        switch (e.obj) {
            case OBJ_CONSTANT: objStr = "constant"; break;
            case OBJ_VARIABLE: objStr = "variable"; break;
            case OBJ_TYPE: objStr = "type"; break;
            case OBJ_PROCEDURE: objStr = "procedure"; break;
            case OBJ_FUNCTION: objStr = "function"; break;
            case OBJ_PROGRAM: objStr = "program"; break;
            case OBJ_FIELD: objStr = "field"; break;
            default: objStr = "-"; break;
        }
        os << std::setw(4) << i << " "
           << std::setw(12) << e.id << " "
           << std::setw(10) << objStr << " "
           << std::setw(6) << e.type << " "
           << std::setw(5) << e.ref << " "
           << std::setw(4) << e.nrm << " "
           << std::setw(4) << e.lev << " "
           << std::setw(4) << e.adr << " "
           << std::setw(5) << e.link << "\n";
    }
}

void SymbolTable::printBTab(std::ostream& os) const {
    os << "\n=== btab ===\n";
    os << std::setw(4) << "idx" << " "
       << std::setw(5) << "last" << " "
       << std::setw(5) << "lpar" << " "
       << std::setw(5) << "psze" << " "
       << std::setw(5) << "vsze" << "\n";
    os << "-----------------------------\n";
    for (size_t i = 0; i < btab.size(); ++i) {
        const auto& e = btab[i];
        os << std::setw(4) << i << " "
           << std::setw(5) << e.last << " "
           << std::setw(5) << e.lpar << " "
           << std::setw(5) << e.psze << " "
           << std::setw(5) << e.vsze << "\n";
    }
}

void SymbolTable::printATab(std::ostream& os) const {
    os << "\n=== atab ===\n";
    if (atab.empty()) {
        os << "(empty)\n";
        return;
    }
    os << std::setw(4) << "idx" << " "
       << std::setw(5) << "xtyp" << " "
       << std::setw(5) << "etyp" << " "
       << std::setw(5) << "eref" << " "
       << std::setw(5) << "low" << " "
       << std::setw(5) << "high" << " "
       << std::setw(5) << "elsz" << " "
       << std::setw(5) << "size" << "\n";
    os << "-----------------------------------------------\n";
    for (size_t i = 0; i < atab.size(); ++i) {
        const auto& e = atab[i];
        os << std::setw(4) << i << " "
           << std::setw(5) << e.xtyp << " "
           << std::setw(5) << e.etyp << " "
           << std::setw(5) << e.eref << " "
           << std::setw(5) << e.low << " "
           << std::setw(5) << e.high << " "
           << std::setw(5) << e.elsz << " "
           << std::setw(5) << e.size << "\n";
    }
}

SemanticAnalyzer::SemanticAnalyzer() {}

void SemanticAnalyzer::analyze(ASTNode* root) {
    if (!root) return;
    visit(root);
}

void SemanticAnalyzer::visit(ASTNode* node) {
    if (!node) return;
    switch (node->nodeType) {
        case AST_PROGRAM:     
            visitProgram(static_cast<ProgramNode*>(node)); break;
        case AST_BLOCK:       
            visitBlock(static_cast<BlockNode*>(node)); break;
        case AST_VARDECL:     
            visitVarDecl(static_cast<VarDeclNode*>(node)); break;
        case AST_CONSTDECL:   
            visitConstDecl(static_cast<ConstDeclNode*>(node)); break;
        case AST_TYPEDECL:    visitTypeDecl(static_cast<TypeDeclNode*>(node)); break;
        case AST_PROCDECL:    visitProcDecl(static_cast<ProcDeclNode*>(node)); break;
        case AST_FUNCDECL:    visitFuncDecl(static_cast<FuncDeclNode*>(node)); break;
        case AST_ASSIGN:      visitAssign(static_cast<AssignNode*>(node)); break;
        case AST_IF:          visitIf(static_cast<IfNode*>(node)); break;
        case AST_WHILE:       visitWhile(static_cast<WhileNode*>(node)); break;
        case AST_FOR:         visitFor(static_cast<ForNode*>(node)); break;
        case AST_REPEAT:      visitRepeat(static_cast<RepeatNode*>(node)); break;
        case AST_CASE:        visitCase(static_cast<CaseNode*>(node)); break;
        case AST_CALL:        visitCall(static_cast<CallNode*>(node)); break;
        case AST_BINOP:       node->dataType = visitBinOp(static_cast<BinOpNode*>(node)); break;
        case AST_UNARYOP:     node->dataType = visitUnaryOp(static_cast<UnaryOpNode*>(node)); break;
        case AST_VAR:         node->dataType = visitVar(static_cast<VarNode*>(node)); break;
        case AST_INDEX:       node->dataType = visitIndex(static_cast<IndexNode*>(node)); break;
        case AST_FIELDACCESS: node->dataType = visitFieldAccess(static_cast<FieldAccessNode*>(node)); break;
        case AST_NUMBER:      node->dataType = visitNumber(static_cast<NumberNode*>(node)); break;
        case AST_STRING:      node->dataType = visitString(static_cast<StringNode*>(node)); break;
        case AST_CHAR:        node->dataType = visitChar(static_cast<CharNode*>(node)); break;
        case AST_BOOL:        node->dataType = visitBool(static_cast<BoolNode*>(node)); break;
        default:

            break;
    }
}

void SemanticAnalyzer::visitProgram(ProgramNode* n) {

    int progIdx = sym.enter(n->name, OBJ_PROGRAM, T_VOID, 0, 1, 0);
    n->tabIndex = progIdx;

    for (ASTNode* decl : n->decls) {
        if (decl->nodeType == AST_BLOCK) {

            BlockNode* blk = static_cast<BlockNode*>(decl);
            for (ASTNode* d : blk->stmts) {
                visit(d);
            }
        } else {
            visit(decl);
        }
    }

    sym.enterBlock(false);
    auto* mainBlock = static_cast<BlockNode*>(n->block);
    mainBlock->blockIndex = sym.currentBTab();
    mainBlock->lexLevel = sym.currentLevel();
    visitBlock(mainBlock);
    sym.exitBlock();
}

void SemanticAnalyzer::visitBlock(BlockNode* n, const std::vector<ASTNode*>& extraDecls) {
    for (ASTNode* decl : extraDecls) {
        visit(decl);
    }
    for (ASTNode* stmt : n->stmts) {
        visitStmt(stmt);
    }
}

void SemanticAnalyzer::visitVarDecl(VarDeclNode* n) {
    int ref = 0;
    DataType dt = visitTypeNode(n->typeNode, ref);
    for (const std::string& name : n->names) {
        int idx = sym.enter(name, OBJ_VARIABLE, dt, ref);
        n->tabIndex = idx; 
    }
}

void SemanticAnalyzer::visitConstDecl(ConstDeclNode* n) {
    DataType valType = visitExpr(n->value);
    int val = 0;
    if (valType == T_INTEGER || valType == T_SUBRANGE) {
        if (n->value->nodeType == AST_NUMBER) val = std::stoi(static_cast<NumberNode*>(n->value)->value);
    } else if (valType == T_BOOLEAN) {
        if (n->value->nodeType == AST_BOOL) val = static_cast<BoolNode*>(n->value)->value ? 1 : 0;
    }
    int idx = sym.enter(n->name, OBJ_CONSTANT, valType, 0, 1, val);
    n->tabIndex = idx;
}

void SemanticAnalyzer::visitTypeDecl(TypeDeclNode* n) {
    int ref = 0;
    DataType dt = visitTypeNode(n->typeNode, ref);
    int idx = sym.enter(n->name, OBJ_TYPE, dt, ref);
    n->tabIndex = idx;

    if (n->typeNode && n->typeNode->nodeType == AST_ENUM) {
        auto* en = static_cast<EnumNode*>(n->typeNode);
        int ordinal = 0;
        for (const std::string& id : en->identifiers) {
            sym.enter(id, OBJ_CONSTANT, dt, ref, 1, ordinal++);
        }
    }
}

void SemanticAnalyzer::visitProcDecl(ProcDeclNode* n) {
    int idx = sym.enter(n->name, OBJ_PROCEDURE, T_VOID, 0, 1, 0);
    n->tabIndex = idx;

    sym.enterBlock(true);
    int blockIdx = sym.currentBTab();
    sym.tab[idx].ref = blockIdx;

    for (ParamNode* p : n->params) {
        visitParam(p);
    }

    for (ASTNode* decl : n->decls) {
        if (decl->nodeType == AST_BLOCK) {
            BlockNode* blk = static_cast<BlockNode*>(decl);
            for (ASTNode* d : blk->stmts) visit(d);
        } else {
            visit(decl);
        }
    }

    if (n->block) {
        auto* blk = static_cast<BlockNode*>(n->block);
        blk->blockIndex = blockIdx;
        blk->lexLevel = sym.currentLevel();
        visitBlock(blk);
    }

    sym.exitBlock();
}

void SemanticAnalyzer::visitFuncDecl(FuncDeclNode* n) {
    int ref = 0;
    DataType retType = visitTypeNode(n->returnType, ref);
    int idx = sym.enter(n->name, OBJ_FUNCTION, retType, 0, 1, 0);
    n->tabIndex = idx;

    sym.enterBlock(true);
    int blockIdx = sym.currentBTab();
    sym.tab[idx].ref = blockIdx;

    for (ParamNode* p : n->params) {
        visitParam(p);
    }

    int funcVarIdx = sym.enter(n->name, OBJ_VARIABLE, retType, 0, 1, 0);
    (void)funcVarIdx;

    for (ASTNode* decl : n->decls) {
        if (decl->nodeType == AST_BLOCK) {
            BlockNode* blk = static_cast<BlockNode*>(decl);
            for (ASTNode* d : blk->stmts) visit(d);
        } else {
            visit(decl);
        }
    }

    if (n->block) {
        auto* blk = static_cast<BlockNode*>(n->block);
        blk->blockIndex = blockIdx;
        blk->lexLevel = sym.currentLevel();
        visitBlock(blk);
    }

    sym.exitBlock();
}

void SemanticAnalyzer::visitParam(ParamNode* n) {
    int ref = 0;
    DataType dt = visitTypeNode(n->typeNode, ref);
    int idx = sym.enterParameter(n->name, dt, ref, n->isVar);
    n->tabIndex = idx;
}

void SemanticAnalyzer::visitFieldDecl(FieldDeclNode* n, int recordBTab) {
    int ref = 0;
    DataType dt = visitTypeNode(n->typeNode, ref);
    for (const std::string& name : n->names) {
        sym.tab.push_back({name, sym.btab[recordBTab].last, OBJ_FIELD, dt, ref, 1, sym.level, 0});
        int idx = static_cast<int>(sym.tab.size()) - 1;
        sym.btab[recordBTab].last = idx;
        n->tabIndex = idx;
    }
}

DataType SemanticAnalyzer::visitTypeNode(ASTNode* n, int& outRef) {
    outRef = 0;
    if (!n) return T_ERROR;
    switch (n->nodeType) {
        case AST_TYPEREF: {
            auto* tr = static_cast<TypeRefNode*>(n);
            int idx = sym.lookup(tr->name);
            if (idx < 0) throw SemanticError("Semantic error: unknown type '" + tr->name + "'");
            if (sym.tab[idx].obj != OBJ_TYPE) throw SemanticError("Semantic error: '" + tr->name + "' is not a type");
            outRef = sym.tab[idx].ref;
            return static_cast<DataType>(sym.tab[idx].type);
        }
        case AST_RANGE:     return visitRangeNode(static_cast<RangeNode*>(n), outRef);
        case AST_ENUM:      return visitEnumNode(static_cast<EnumNode*>(n), outRef);
        case AST_ARRAYTYPE: return visitArrayType(static_cast<ArrayTypeNode*>(n), outRef);
        case AST_RECORDTYPE: return visitRecordType(static_cast<RecordTypeNode*>(n), outRef);
        default: return T_ERROR;
    }
}

DataType SemanticAnalyzer::visitRangeNode(RangeNode* n, int& outRef) {
    DataType lowType = visitExpr(n->low);
    DataType highType = visitExpr(n->high);
    if (lowType != highType) {
        throw SemanticError("Semantic error: range bounds must have the same type");
    }
    if (lowType == T_REAL) {
        throw SemanticError("Semantic error: subrange cannot be of type real");
    }
    if (lowType != T_INTEGER && lowType != T_CHAR && lowType != T_BOOLEAN) {
        throw SemanticError("Semantic error: subrange bounds must be integer, char, or boolean");
    }

    if (n->low->nodeType == AST_NUMBER && n->high->nodeType == AST_NUMBER) {
        int lo = std::stoi(static_cast<NumberNode*>(n->low)->value);
        int hi = std::stoi(static_cast<NumberNode*>(n->high)->value);
        if (lo > hi) throw SemanticError("Semantic error: lower bound exceeds upper bound in subrange");
    }

    int aidx = static_cast<int>(sym.atab.size());
    int elsz = 1;
    int lowVal = 0, highVal = 0;
    if (n->low->nodeType == AST_NUMBER) lowVal = std::stoi(static_cast<NumberNode*>(n->low)->value);
    if (n->high->nodeType == AST_NUMBER) highVal = std::stoi(static_cast<NumberNode*>(n->high)->value);
    sym.atab.push_back({lowType, lowType, 0, lowVal, highVal, elsz, (highVal - lowVal + 1) * elsz});
    outRef = aidx;
    return T_SUBRANGE;
}

DataType SemanticAnalyzer::visitEnumNode(EnumNode* n, int& outRef) {

    int aidx = static_cast<int>(sym.atab.size());
    int count = static_cast<int>(n->identifiers.size());
    sym.atab.push_back({T_INTEGER, T_INTEGER, 0, 0, count - 1, 1, count});
    outRef = aidx;
    return T_ENUM;
}

DataType SemanticAnalyzer::visitArrayType(ArrayTypeNode* n, int& outRef) {
    int idxRef = 0, elRef = 0;
    DataType idxType = visitTypeNode(n->indexType, idxRef);
    if (idxType == T_REAL || idxType == T_ARRAY || idxType == T_RECORD) {
        throw SemanticError("Semantic error: array index type must be a simple non-real type");
    }
    DataType elType = visitTypeNode(n->elementType, elRef);

    int low = 0, high = 0, elsz = 1;
    if (idxType == T_SUBRANGE && idxRef >= 0 && idxRef < static_cast<int>(sym.atab.size())) {
        low = sym.atab[idxRef].low;
        high = sym.atab[idxRef].high;
    } else if (idxType == T_INTEGER) {
        low = 0; high = 65535; 
    } else if (idxType == T_CHAR) {
        low = 0; high = 255;
    } else if (idxType == T_BOOLEAN) {
        low = 0; high = 1;
    }

    int aidx = static_cast<int>(sym.atab.size());
    sym.atab.push_back({idxType, elType, elRef, low, high, elsz, (high - low + 1) * elsz});
    outRef = aidx;
    return T_ARRAY;
}

DataType SemanticAnalyzer::visitRecordType(RecordTypeNode* n, int& outRef) {
    int bidx = static_cast<int>(sym.btab.size());
    sym.btab.push_back({0, 0, 0, 0});
    outRef = bidx;
    for (FieldDeclNode* f : n->fields) {
        visitFieldDecl(f, bidx);
    }
    return T_RECORD;
}

void SemanticAnalyzer::visitStmt(ASTNode* n) {
    if (!n) return;
    switch (n->nodeType) {
        case AST_ASSIGN:   visitAssign(static_cast<AssignNode*>(n)); break;
        case AST_IF:       visitIf(static_cast<IfNode*>(n)); break;
        case AST_WHILE:    visitWhile(static_cast<WhileNode*>(n)); break;
        case AST_FOR:      visitFor(static_cast<ForNode*>(n)); break;
        case AST_REPEAT:   visitRepeat(static_cast<RepeatNode*>(n)); break;
        case AST_CASE:     visitCase(static_cast<CaseNode*>(n)); break;
        case AST_CALL:     visitCall(static_cast<CallNode*>(n)); break;
        case AST_BLOCK:    visitBlock(static_cast<BlockNode*>(n)); break;
        default: break;
    }
}

void SemanticAnalyzer::visitAssign(AssignNode* n) {
    DataType targetType = T_ERROR;
    int targetRef = 0;
    int targetIdx = -1;

    if (n->target->nodeType == AST_VAR) {
        targetType = visitVar(static_cast<VarNode*>(n->target));
        targetIdx = n->target->tabIndex;
        targetRef = (targetIdx >= 0) ? sym.tab[targetIdx].ref : 0;
    } else if (n->target->nodeType == AST_INDEX) {
        targetType = visitIndex(static_cast<IndexNode*>(n->target));
    } else if (n->target->nodeType == AST_FIELDACCESS) {
        targetType = visitFieldAccess(static_cast<FieldAccessNode*>(n->target));
    }

    DataType valType = visitExpr(n->value);
    int valRef = 0;
    if (n->value->tabIndex >= 0) valRef = sym.tab[n->value->tabIndex].ref;

    if (!sym.assignmentCompatible(targetType, targetRef, valType, valRef)) {
        throw SemanticError("Semantic error: assignment type mismatch (cannot assign " +
                            std::string(dataTypeName(valType)) + " to " + std::string(dataTypeName(targetType)) + ")");
    }
    n->dataType = T_VOID;
}

void SemanticAnalyzer::visitIf(IfNode* n) {
    DataType cond = visitExpr(n->condition);
    if (cond != T_BOOLEAN) {
        throw SemanticError("Semantic error: if condition must be boolean");
    }
    visitStmt(n->thenBranch);
    if (n->elseBranch) visitStmt(n->elseBranch);
}

void SemanticAnalyzer::visitWhile(WhileNode* n) {
    DataType cond = visitExpr(n->condition);
    if (cond != T_BOOLEAN) {
        throw SemanticError("Semantic error: while condition must be boolean");
    }
    if (n->body) visitBlock(static_cast<BlockNode*>(n->body));
}

void SemanticAnalyzer::visitFor(ForNode* n) {

    int idx = sym.lookup(n->varName);
    if (idx < 0) throw SemanticError("Semantic error: for loop variable '" + n->varName + "' is not declared");
    if (sym.tab[idx].obj != OBJ_VARIABLE) throw SemanticError("Semantic error: for loop variable must be a variable");
    DataType varType = static_cast<DataType>(sym.tab[idx].type);
    if (varType != T_INTEGER && varType != T_SUBRANGE && varType != T_CHAR && varType != T_BOOLEAN) {
        throw SemanticError("Semantic error: for loop variable must be ordinal type");
    }
    n->init->tabIndex = idx; 

    DataType initType = visitExpr(n->init);
    DataType endType = visitExpr(n->endExpr);
    if (!sym.compatible(varType, initType) || !sym.compatible(varType, endType)) {
        throw SemanticError("Semantic error: for loop bounds type mismatch");
    }
    if (n->body) visitBlock(static_cast<BlockNode*>(n->body));
}

void SemanticAnalyzer::visitRepeat(RepeatNode* n) {
    for (ASTNode* s : n->stmts) visitStmt(s);
    DataType cond = visitExpr(n->condition);
    if (cond != T_BOOLEAN) {
        throw SemanticError("Semantic error: repeat condition must be boolean");
    }
}

void SemanticAnalyzer::visitCase(CaseNode* n) {
    DataType exprType = visitExpr(n->expr);
    for (ASTNode* arm : n->arms) {
        auto* ca = static_cast<CaseArmNode*>(arm);
        for (ASTNode* c : ca->constants) {
            DataType ct = visitExpr(c);
            if (!sym.compatible(exprType, ct)) {
                throw SemanticError("Semantic error: case constant type mismatch");
            }
        }
        visitStmt(ca->stmt);
    }
}

void SemanticAnalyzer::visitCall(CallNode* n) {
    int idx = sym.lookup(n->name);
    if (idx < 0) {
        throw SemanticError("Semantic error: undeclared procedure/function '" + n->name + "'");
    }
    n->tabIndex = idx;
    n->lexLevel = sym.tab[idx].lev;

    int obj = sym.tab[idx].obj;
    if (obj != OBJ_PROCEDURE && obj != OBJ_FUNCTION) {
        throw SemanticError("Semantic error: '" + n->name + "' is not a procedure or function");
    }

    n->dataType = (obj == OBJ_FUNCTION) ? static_cast<DataType>(sym.tab[idx].type) : T_VOID;

    if (n->name == "readln" || n->name == "writeln") {

        for (ASTNode* arg : n->args) visitExpr(arg);
        return;
    }

    int blockRef = sym.tab[idx].ref;
    if (blockRef < 0 || blockRef >= static_cast<int>(sym.btab.size())) return;

    int paramCount = 0;
    int p = sym.btab[blockRef].lpar;
    while (p != 0) {
        ++paramCount;
        p = sym.tab[p].link;
    }

    if (static_cast<int>(n->args.size()) != paramCount) {
        throw SemanticError("Semantic error: argument count mismatch in call to '" + n->name + "'");
    }

    std::vector<int> paramTypes;
    p = sym.btab[blockRef].lpar;
    while (p != 0) {
        paramTypes.push_back(p);
        p = sym.tab[p].link;
    }
    std::reverse(paramTypes.begin(), paramTypes.end());

    for (size_t i = 0; i < n->args.size(); ++i) {
        DataType argType = visitExpr(n->args[i]);
        int pidx = paramTypes[i];
        int expectedType = sym.tab[pidx].type;
        int expectedRef = sym.tab[pidx].ref;
        if (!sym.assignmentCompatible(expectedType, expectedRef, argType, 0)) {
            throw SemanticError("Semantic error: argument type mismatch in call to '" + n->name + "'");
        }
    }
}

DataType SemanticAnalyzer::visitExpr(ASTNode* n) {
    if (!n) return T_ERROR;
    switch (n->nodeType) {
        case AST_BINOP:       return visitBinOp(static_cast<BinOpNode*>(n));
        case AST_UNARYOP:     return visitUnaryOp(static_cast<UnaryOpNode*>(n));
        case AST_VAR:         return visitVar(static_cast<VarNode*>(n));
        case AST_INDEX:       return visitIndex(static_cast<IndexNode*>(n));
        case AST_FIELDACCESS: return visitFieldAccess(static_cast<FieldAccessNode*>(n));
        case AST_NUMBER:      return visitNumber(static_cast<NumberNode*>(n));
        case AST_STRING:      return visitString(static_cast<StringNode*>(n));
        case AST_CHAR:        return visitChar(static_cast<CharNode*>(n));
        case AST_BOOL:        return visitBool(static_cast<BoolNode*>(n));
        case AST_CALL:        visitCall(static_cast<CallNode*>(n)); return n->dataType;
        case AST_RANGE:       { int ref; return visitRangeNode(static_cast<RangeNode*>(n), ref); }
        case AST_ENUM:        { int ref; return visitEnumNode(static_cast<EnumNode*>(n), ref); }
        default: return T_ERROR;
    }
}

DataType SemanticAnalyzer::visitBinOp(BinOpNode* n) {
    DataType left = visitExpr(n->left);
    DataType right = visitExpr(n->right);
    std::string op = n->op;

    if (op == "==" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (!sym.compatible(left, right)) {
            throw SemanticError("Semantic error: incompatible types in relational operation");
        }
        n->dataType = T_BOOLEAN;
        return T_BOOLEAN;
    }

    if (op == "and" || op == "or") {
        if (left != T_BOOLEAN || right != T_BOOLEAN) {
            throw SemanticError("Semantic error: boolean operators require boolean operands");
        }
        n->dataType = T_BOOLEAN;
        return T_BOOLEAN;
    }

    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!sym.isNumericType(left) || !sym.isNumericType(right)) {
            throw SemanticError("Semantic error: arithmetic operators require numeric operands");
        }
        if (left == T_REAL || right == T_REAL) {
            n->dataType = T_REAL;
            return T_REAL;
        }
        n->dataType = T_INTEGER;
        return T_INTEGER;
    }

    if (op == "div" || op == "mod") {
        if ((left != T_INTEGER && left != T_SUBRANGE) || (right != T_INTEGER && right != T_SUBRANGE)) {
            throw SemanticError("Semantic error: div/mod require integer operands");
        }
        n->dataType = T_INTEGER;
        return T_INTEGER;
    }

    n->dataType = T_ERROR;
    return T_ERROR;
}

DataType SemanticAnalyzer::visitUnaryOp(UnaryOpNode* n) {
    DataType operand = visitExpr(n->operand);
    if (n->op == "not") {
        if (operand != T_BOOLEAN) throw SemanticError("Semantic error: 'not' requires boolean operand");
        n->dataType = T_BOOLEAN;
        return T_BOOLEAN;
    }
    if (n->op == "+" || n->op == "-") {
        if (!sym.isNumericType(operand)) throw SemanticError("Semantic error: unary +/- requires numeric operand");
        n->dataType = operand;
        return operand;
    }
    return T_ERROR;
}

DataType SemanticAnalyzer::visitVar(VarNode* n) {
    int idx = sym.lookup(n->name);
    if (idx < 0) {
        throw SemanticError("Semantic error: undeclared identifier '" + n->name + "'");
    }
    n->tabIndex = idx;
    n->lexLevel = sym.tab[idx].lev;
    n->dataType = static_cast<DataType>(sym.tab[idx].type);
    return n->dataType;
}

DataType SemanticAnalyzer::visitIndex(IndexNode* n) {
    DataType baseType = visitExpr(n->base);
    if (baseType != T_ARRAY) {
        throw SemanticError("Semantic error: indexing non-array type");
    }
    int baseIdx = n->base->tabIndex;
    if (baseIdx < 0) {
        throw SemanticError("Semantic error: cannot resolve array type for indexing");
    }
    int aRef = sym.tab[baseIdx].ref;
    if (aRef < 0 || aRef >= static_cast<int>(sym.atab.size())) {
        throw SemanticError("Semantic error: invalid array reference");
    }
    DataType idxType = static_cast<DataType>(sym.atab[aRef].xtyp);
    for (ASTNode* ix : n->indices) {
        DataType it = visitExpr(ix);
        if (!sym.compatible(idxType, it)) {
            throw SemanticError("Semantic error: array index type mismatch");
        }
    }
    DataType elType = static_cast<DataType>(sym.atab[aRef].etyp);
    n->dataType = elType;
    return elType;
}

DataType SemanticAnalyzer::visitFieldAccess(FieldAccessNode* n) {
    DataType baseType = visitExpr(n->base);
    if (baseType != T_RECORD) {
        throw SemanticError("Semantic error: field access on non-record type");
    }
    int baseIdx = n->base->tabIndex;
    if (baseIdx < 0) {
        throw SemanticError("Semantic error: cannot resolve record type");
    }
    int bRef = sym.tab[baseIdx].ref;
    if (bRef < 0 || bRef >= static_cast<int>(sym.btab.size())) {
        throw SemanticError("Semantic error: invalid record reference");
    }

    int p = sym.btab[bRef].last;
    while (p != 0) {
        if (sym.tab[p].id == n->field) {
            n->tabIndex = p;
            n->dataType = static_cast<DataType>(sym.tab[p].type);
            return n->dataType;
        }
        p = sym.tab[p].link;
    }
    throw SemanticError("Semantic error: unknown field '" + n->field + "' in record");
}

DataType SemanticAnalyzer::visitNumber(NumberNode* n) {
    n->dataType = n->isReal ? T_REAL : T_INTEGER;
    return n->dataType;
}

DataType SemanticAnalyzer::visitString(StringNode* n) {
    n->dataType = T_STRING;
    return T_STRING;
}

DataType SemanticAnalyzer::visitChar(CharNode* n) {
    n->dataType = T_CHAR;
    return T_CHAR;
}

DataType SemanticAnalyzer::visitBool(BoolNode* n) {
    n->dataType = T_BOOLEAN;
    return T_BOOLEAN;
}

void SemanticAnalyzer::expectType(DataType got, DataType expected, const std::string& ctx) {
    if (got != expected) {
        throw SemanticError("Semantic error: expected " + std::string(dataTypeName(expected)) +
                            " but got " + std::string(dataTypeName(got)) + " in " + ctx);
    }
}

void SemanticAnalyzer::expectNumeric(DataType got, const std::string& ctx) {
    if (!sym.isNumericType(got)) {
        throw SemanticError("Semantic error: expected numeric type in " + ctx);
    }
}

void SemanticAnalyzer::expectBoolean(DataType got, const std::string& ctx) {
    if (got != T_BOOLEAN) {
        throw SemanticError("Semantic error: expected boolean in " + ctx);
    }
}