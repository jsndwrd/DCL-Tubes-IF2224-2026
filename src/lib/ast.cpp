// Implementasi AST builder dan print helper
#include "../header/ast.hpp"
#include "../header/parser.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

static std::string stripIdent(const std::string& label) {

    if (label.rfind("ident(", 0) == 0) {
        std::string inner = label.substr(6);
        if (!inner.empty() && inner.back() == ')') inner.pop_back();
        return inner;
    }
    return label;
}

static std::string stripTokenValue(const std::string& label) {

    size_t p = label.find('(');
    if (p != std::string::npos) {
        std::string inner = label.substr(p + 1);
        if (!inner.empty() && inner.back() == ')') inner.pop_back();
        return inner;
    }
    return label;
}

const char* dataTypeName(DataType dt) {
    switch (dt) {
        case T_INTEGER:  return "integer";
        case T_REAL:     return "real";
        case T_CHAR:     return "char";
        case T_BOOLEAN:  return "boolean";
        case T_STRING:   return "string";
        case T_SUBRANGE: return "subrange";
        case T_ENUM:     return "enumerated";
        case T_ARRAY:    return "array";
        case T_RECORD:   return "record";
        case T_VOID:     return "void";
        case T_ERROR:    return "error";
        default:         return "unknown";
    }
}

const char* astNodeTypeName(ASTNodeType nt) {
    switch (nt) {
        case AST_PROGRAM:     return "Program";
        case AST_BLOCK:       return "Block";
        case AST_VARDECL:     return "VarDecl";
        case AST_CONSTDECL:   return "ConstDecl";
        case AST_TYPEDECL:    return "TypeDecl";
        case AST_PROCDECL:    return "ProcDecl";
        case AST_FUNCDECL:    return "FuncDecl";
        case AST_FIELDDECL:   return "FieldDecl";
        case AST_PARAM:       return "Param";
        case AST_ASSIGN:      return "Assign";
        case AST_IF:          return "If";
        case AST_WHILE:       return "While";
        case AST_FOR:         return "For";
        case AST_REPEAT:      return "Repeat";
        case AST_CASE:        return "Case";
        case AST_CASEARM:     return "CaseArm";
        case AST_BINOP:       return "BinOp";
        case AST_UNARYOP:     return "UnaryOp";
        case AST_CALL:        return "Call";
        case AST_VAR:         return "Var";
        case AST_INDEX:       return "Index";
        case AST_FIELDACCESS: return "FieldAccess";
        case AST_NUMBER:      return "Number";
        case AST_STRING:      return "String";
        case AST_CHAR:        return "Char";
        case AST_BOOL:        return "Bool";
        case AST_RANGE:       return "Range";
        case AST_ENUM:        return "Enum";
        case AST_ARRAYTYPE:   return "ArrayType";
        case AST_RECORDTYPE:  return "RecordType";
        case AST_TYPEREF:     return "TypeRef";
        default:              return "Unknown";
    }
}

std::string ASTNode::label() const {
    return astNodeTypeName(nodeType);
}

static void printAnnotation(std::ostream& os, const ASTNode* n) {
    bool hasAnno = false;
    if (n->tabIndex >= 0) {
        os << " → tab_index:" << n->tabIndex;
        hasAnno = true;
    }
    if (n->dataType != T_VOID) {
        os << ", type:" << dataTypeName(n->dataType);
        hasAnno = true;
    }
    if (n->lexLevel > 0) {
        os << ", lev:" << n->lexLevel;
        hasAnno = true;
    }
    if (!hasAnno && n->nodeType == AST_BLOCK) {
        auto* bn = static_cast<const BlockNode*>(n);
        if (bn->blockIndex >= 0) {
            os << " → block_index:" << bn->blockIndex << ", lev:" << n->lexLevel;
        }
    }
}

void ASTNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    if (isRoot) {
        os << this->label();
        printAnnotation(os, this);
        os << '\n';
    } else {
        os << prefix << (isLast ? "\\-- " : "+-- ") << this->label();
        printAnnotation(os, this);
        os << '\n';
    }
}

void destroyAST(ASTNode* root) {
    delete root; 
}

std::string ProgramNode::label() const {
    return std::string("ProgramNode(name: '") + name + "')";
}

void ProgramNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (size_t i = 0; i < decls.size(); ++i) {
        bool last = (i + 1 == decls.size() && !block);
        if (decls[i]) decls[i]->print(os, np, last, false);
    }
    if (block) block->print(os, np, true, false);
}

std::string BlockNode::label() const {
    return "Block";
}

void BlockNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (size_t i = 0; i < stmts.size(); ++i) {
        if (stmts[i]) stmts[i]->print(os, np, i + 1 == stmts.size(), false);
    }
}

std::string VarDeclNode::label() const {
    std::string s = "VarDecl(";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i) s += ", ";
        s += names[i];
    }
    s += ")";
    return s;
}

void VarDeclNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (typeNode) typeNode->print(os, np, true, false);
}

std::string ConstDeclNode::label() const {
    return std::string("ConstDecl(") + name + ")";
}

void ConstDeclNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (value) value->print(os, np, true, false);
}

std::string TypeDeclNode::label() const {
    return std::string("TypeDecl(") + name + ")";
}

void TypeDeclNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (typeNode) typeNode->print(os, np, true, false);
}

std::string ParamNode::label() const {
    return std::string("Param(") + name + (isVar ? ", var" : "") + ")";
}

void ParamNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (typeNode) typeNode->print(os, np, true, false);
}

std::string ProcDeclNode::label() const {
    return std::string("ProcDecl(") + name + ")";
}

void ProcDeclNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i]) params[i]->print(os, np, i + 1 == params.size() && decls.empty() && !block, false);
    }
    for (size_t i = 0; i < decls.size(); ++i) {
        if (decls[i]) decls[i]->print(os, np, i + 1 == decls.size() && !block, false);
    }
    if (block) block->print(os, np, true, false);
}

std::string FuncDeclNode::label() const {
    return std::string("FuncDecl(") + name + ")";
}

void FuncDeclNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i]) params[i]->print(os, np, i + 1 == params.size() && decls.empty() && !returnType && !block, false);
    }
    for (size_t i = 0; i < decls.size(); ++i) {
        if (decls[i]) decls[i]->print(os, np, i + 1 == decls.size() && !returnType && !block, false);
    }
    if (returnType) returnType->print(os, np, !block, false);
    if (block) block->print(os, np, true, false);
}

std::string FieldDeclNode::label() const {
    std::string s = "FieldDecl(";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i) s += ", ";
        s += names[i];
    }
    s += ")";
    return s;
}

std::string AssignNode::label() const {
    return "Assign";
}

void AssignNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (target) target->print(os, np, false, false);
    if (value) value->print(os, np, true, false);
}

std::string IfNode::label() const {
    return "If";
}

void IfNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (condition) condition->print(os, np, false, false);
    if (thenBranch) thenBranch->print(os, np, !elseBranch, false);
    if (elseBranch) elseBranch->print(os, np, true, false);
}

std::string WhileNode::label() const {
    return "While";
}

std::string ForNode::label() const {
    return std::string("For(") + varName + ", " + (isTo ? "to" : "downto") + ")";
}

std::string RepeatNode::label() const {
    return "Repeat";
}

std::string CaseNode::label() const {
    return "Case";
}

std::string CaseArmNode::label() const {
    return "CaseArm";
}

std::string BinOpNode::label() const {
    return std::string("BinOp(op: '") + op + "')";
}

void BinOpNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    if (left) left->print(os, np, false, false);
    if (right) right->print(os, np, true, false);
}

std::string UnaryOpNode::label() const {
    return std::string("UnaryOp(") + op + ")";
}

std::string CallNode::label() const {
    return std::string("ProcedureCall(name: '") + name + "')";
}

void CallNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
    std::string np = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i]) args[i]->print(os, np, i + 1 == args.size(), false);
    }
}

std::string VarNode::label() const {
    return std::string("Var(") + name + ")";
}

void VarNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
}

std::string IndexNode::label() const {
    return "Index";
}

std::string FieldAccessNode::label() const {
    return std::string("FieldAccess(") + field + ")";
}

std::string NumberNode::label() const {
    return std::string("Num(") + value + ")";
}

void NumberNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
}

std::string StringNode::label() const {
    return std::string("Str(") + value + ")";
}

void StringNode::print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const {
    ASTNode::print(os, prefix, isLast, isRoot);
}

std::string CharNode::label() const {
    return std::string("Char(") + value + ")";
}

std::string BoolNode::label() const {
    return std::string("Bool(") + (value ? "true" : "false") + ")";
}

std::string RangeNode::label() const {
    return "Range";
}

std::string EnumNode::label() const {
    return "Enum";
}

std::string ArrayTypeNode::label() const {
    return "ArrayType";
}

std::string RecordTypeNode::label() const {
    return "RecordType";
}

std::string TypeRefNode::label() const {
    return std::string("TypeRef(") + name + ")";
}

static ASTNode* buildProgram(ParseNode* n);
static ASTNode* buildHeader(ParseNode* n);
static std::vector<ASTNode*> buildDeclarations(ParseNode* n);
static ASTNode* buildConsts(ParseNode* n);
static ASTNode* buildTypes(ParseNode* n);
static ASTNode* buildVars(ParseNode* n);
static ASTNode* buildSub(ParseNode* n);
static ASTNode* buildProc(ParseNode* n);
static ASTNode* buildFunc(ParseNode* n);
static std::vector<ParamNode*> buildFormals(ParseNode* n);
static std::vector<ParamNode*> buildParamGroup(ParseNode* n);
static ASTNode* buildType(ParseNode* n);
static ASTNode* buildArrayType(ParseNode* n);
static ASTNode* buildRecordType(ParseNode* n);
static ASTNode* buildRange(ParseNode* n);
static ASTNode* buildEnum(ParseNode* n);
static ASTNode* buildCompound(ParseNode* n);
static std::vector<ASTNode*> buildStmtList(ParseNode* n);
static ASTNode* buildStmt(ParseNode* n);
static ASTNode* buildAssign(ParseNode* n);
static ASTNode* buildIf(ParseNode* n);
static ASTNode* buildCase(ParseNode* n);
static ASTNode* buildCaseArm(ParseNode* n);
static ASTNode* buildWhile(ParseNode* n);
static ASTNode* buildRepeat(ParseNode* n);
static ASTNode* buildFor(ParseNode* n);
static ASTNode* buildCall(ParseNode* n);
static ASTNode* buildExpr(ParseNode* n);
static ASTNode* buildSimple(ParseNode* n);
static ASTNode* buildTerm(ParseNode* n);
static ASTNode* buildFactor(ParseNode* n);
static ASTNode* buildVariable(ParseNode* n);

ASTNode* buildAST(ParseNode* root) {
    if (!root || root->label != "<program>") return nullptr;
    return buildProgram(root);
}

static ASTNode* buildProgram(ParseNode* n) {
    if (n->children.size() < 4) return nullptr;
    auto* prog = new ProgramNode("");
    prog->name = buildHeader(n->children[0]) ? static_cast<ProgramNode*>(prog)->name : "";

    ParseNode* hdr = n->children[0];
    if (hdr && hdr->children.size() >= 2) {
        prog->name = stripIdent(hdr->children[1]->label);
    }
    prog->decls = buildDeclarations(n->children[1]);
    prog->block = buildCompound(n->children[2]);
    return prog;
}

static ASTNode* buildHeader(ParseNode* n) {
    return nullptr; 
}

static std::vector<ASTNode*> buildDeclarations(ParseNode* n) {
    std::vector<ASTNode*> decls;
    if (!n || n->label != "<declaration-part>") return decls;
    for (ParseNode* child : n->children) {
        if (!child) continue;
        if (child->label == "<const-declaration>") {
            auto* c = buildConsts(child);
            if (c) decls.push_back(c);
        } else if (child->label == "<type-declaration>") {
            auto* t = buildTypes(child);
            if (t) decls.push_back(t);
        } else if (child->label == "<var-declaration>") {
            auto* v = buildVars(child);
            if (v) decls.push_back(v);
        } else if (child->label == "<subprogram-declaration>") {
            auto* s = buildSub(child);
            if (s) decls.push_back(s);
        }
    }
    return decls;
}

static ASTNode* buildConsts(ParseNode* n) {
    if (!n || n->label != "<const-declaration>") return nullptr;

    auto* blk = new BlockNode();
    size_t i = 1; 
    while (i < n->children.size()) {
        if (!n->children[i]) { ++i; continue; }
        std::string id = stripIdent(n->children[i]->label);
        ++i; 
        if (i < n->children.size() && n->children[i]->label.rfind("eql", 0) == 0) ++i; 
        ASTNode* val = nullptr;
        if (i < n->children.size() && n->children[i]->label == "<constant>") {
            val = buildFactor(n->children[i]); 

            ParseNode* c = n->children[i];
            if (c->children.empty()) { val = nullptr; }
            else {
                size_t ci = 0;
                std::string sign;
                if (c->children[ci]->label.rfind("plus", 0) == 0 || c->children[ci]->label.rfind("minus", 0) == 0) {
                    sign = c->children[ci]->label.rfind("plus", 0) == 0 ? "+" : "-";
                    ++ci;
                }
                if (ci < c->children.size()) {
                    std::string lit = c->children[ci]->label;
                    if (lit.rfind("intcon(", 0) == 0) {
                        val = new NumberNode(sign + stripTokenValue(lit), false);
                    } else if (lit.rfind("realcon(", 0) == 0) {
                        val = new NumberNode(sign + stripTokenValue(lit), true);
                    } else if (lit.rfind("charcon(", 0) == 0) {
                        val = new CharNode(stripTokenValue(lit));
                    } else if (lit.rfind("string(", 0) == 0) {
                        val = new StringNode(stripTokenValue(lit));
                    } else if (lit.rfind("ident(", 0) == 0) {
                        val = new VarNode(stripTokenValue(lit));
                    }
                }
            }
            ++i;
        }
        if (!id.empty() && val) {
            auto* cd = new ConstDeclNode(id);
            cd->value = val;
            blk->stmts.push_back(cd);
        }
        if (i < n->children.size() && n->children[i]->label.rfind("semicolon", 0) == 0) ++i;
    }
    return blk;
}

static ASTNode* buildTypes(ParseNode* n) {
    if (!n || n->label != "<type-declaration>") return nullptr;
    auto* blk = new BlockNode();
    size_t i = 1; 
    while (i < n->children.size()) {
        if (!n->children[i]) { ++i; continue; }
        std::string id = stripIdent(n->children[i]->label);
        ++i; 
        if (i < n->children.size() && n->children[i]->label.rfind("eql", 0) == 0) ++i;
        ASTNode* ty = nullptr;
        if (i < n->children.size() && n->children[i]->label == "<type>") {
            ty = buildType(n->children[i]);
            ++i;
        }
        if (!id.empty() && ty) {
            auto* td = new TypeDeclNode(id);
            td->typeNode = ty;
            blk->stmts.push_back(td);
        }
        if (i < n->children.size() && n->children[i]->label.rfind("semicolon", 0) == 0) ++i;
    }
    return blk;
}

static ASTNode* buildVars(ParseNode* n) {
    if (!n || n->label != "<var-declaration>") return nullptr;
    auto* blk = new BlockNode();
    size_t i = 1; 
    while (i < n->children.size()) {
        if (!n->children[i]) { ++i; continue; }
        std::vector<std::string> ids;
        if (n->children[i]->label == "<identifier-list>") {
            for (ParseNode* ic : n->children[i]->children) {
                if (ic->label.rfind("ident(", 0) == 0) ids.push_back(stripIdent(ic->label));
            }
            ++i;
        }
        if (i < n->children.size() && n->children[i]->label.rfind("colon", 0) == 0) ++i;
        ASTNode* ty = nullptr;
        if (i < n->children.size() && n->children[i]->label == "<type>") {
            ty = buildType(n->children[i]);
            ++i;
        }
        if (!ids.empty() && ty) {
            auto* vd = new VarDeclNode();
            vd->names = ids;
            vd->typeNode = ty;
            blk->stmts.push_back(vd);
        }
        if (i < n->children.size() && n->children[i]->label.rfind("semicolon", 0) == 0) ++i;
    }
    return blk;
}

static ASTNode* buildSub(ParseNode* n) {
    if (!n || n->label != "<subprogram-declaration>" || n->children.empty()) return nullptr;
    ParseNode* child = n->children[0];
    if (child->label == "<procedure-declaration>") return buildProc(child);
    if (child->label == "<function-declaration>") return buildFunc(child);
    return nullptr;
}

static ASTNode* buildProc(ParseNode* n) {
    if (!n || n->label != "<procedure-declaration>") return nullptr;
    size_t i = 1;
    std::string name = stripIdent(n->children[i]->label);
    ++i;
    std::vector<ParamNode*> params;
    if (i < n->children.size() && n->children[i]->label == "<formal-parameter-list>") {
        params = buildFormals(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("semicolon", 0) == 0) ++i;
    auto* pd = new ProcDeclNode(name);
    pd->params = params;
    if (i < n->children.size() && n->children[i]->label == "<block>") {
        ParseNode* blk = n->children[i];
        if (blk->children.size() >= 2) {
            pd->decls = buildDeclarations(blk->children[0]);
            pd->block = buildCompound(blk->children[1]);
        }
        ++i;
    }

    return pd;
}

static ASTNode* buildFunc(ParseNode* n) {
    if (!n || n->label != "<function-declaration>") return nullptr;
    size_t i = 1;
    std::string name = stripIdent(n->children[i]->label);
    ++i;
    std::vector<ParamNode*> params;
    if (i < n->children.size() && n->children[i]->label == "<formal-parameter-list>") {
        params = buildFormals(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("colon", 0) == 0) ++i;
    ASTNode* retType = nullptr;
    if (i < n->children.size() && n->children[i]->label.rfind("ident(", 0) == 0) {
        retType = new TypeRefNode(stripIdent(n->children[i]->label));
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("semicolon", 0) == 0) ++i;
    auto* fd = new FuncDeclNode(name);
    fd->params = params;
    fd->returnType = retType;
    if (i < n->children.size() && n->children[i]->label == "<block>") {
        ParseNode* blk = n->children[i];
        if (blk->children.size() >= 2) {
            fd->decls = buildDeclarations(blk->children[0]);
            fd->block = buildCompound(blk->children[1]);
        }
        ++i;
    }
    return fd;
}

static std::vector<ParamNode*> buildFormals(ParseNode* n) {
    std::vector<ParamNode*> params;
    if (!n || n->label != "<formal-parameter-list>") return params;
    for (ParseNode* child : n->children) {
        if (child->label == "<parameter-group>") {
            auto pg = buildParamGroup(child);
            params.insert(params.end(), pg.begin(), pg.end());
        }
    }
    return params;
}

static std::vector<ParamNode*> buildParamGroup(ParseNode* n) {
    std::vector<ParamNode*> params;
    if (!n || n->label != "<parameter-group>") return params;
    size_t i = 0;
    bool isVar = false;
    if (i < n->children.size() && n->children[i]->label.rfind("varsy", 0) == 0) {
        isVar = true;
        ++i;
    }
    std::vector<std::string> ids;
    if (i < n->children.size() && n->children[i]->label == "<identifier-list>") {
        for (ParseNode* ic : n->children[i]->children) {
            if (ic->label.rfind("ident(", 0) == 0) ids.push_back(stripIdent(ic->label));
        }
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("colon", 0) == 0) ++i;
    ASTNode* ty = nullptr;
    if (i < n->children.size()) {
        if (n->children[i]->label == "<array-type>") {
            ty = buildArrayType(n->children[i]);
        } else if (n->children[i]->label.rfind("ident(", 0) == 0) {
            ty = new TypeRefNode(stripIdent(n->children[i]->label));
        }
        ++i;
    }
    for (const std::string& id : ids) {
        auto* p = new ParamNode(id);
        p->isVar = isVar;
        p->typeNode = ty; 

        if (ty && ty->nodeType == AST_TYPEREF) {
            p->typeNode = new TypeRefNode(static_cast<TypeRefNode*>(ty)->name);
        } else if (ty && ty->nodeType == AST_ARRAYTYPE) {

            p->typeNode = ty; 
            ty = nullptr; 
        } else {
            p->typeNode = ty;
            ty = nullptr;
        }
        params.push_back(p);
    }

    if (ty) delete ty;
    return params;
}

static ASTNode* buildType(ParseNode* n) {
    if (!n || n->label != "<type>" || n->children.empty()) return nullptr;
    ParseNode* child = n->children[0];
    if (child->label == "<array-type>") return buildArrayType(child);
    if (child->label == "<record-type>") return buildRecordType(child);
    if (child->label == "<enumerated>") return buildEnum(child);
    if (child->label == "<range>") return buildRange(child);
    if (child->label.rfind("ident(", 0) == 0) return new TypeRefNode(stripIdent(child->label));
    return nullptr;
}

static ASTNode* buildArrayType(ParseNode* n) {
    if (!n || n->label != "<array-type>") return nullptr;
    size_t i = 1; 
    if (i < n->children.size() && n->children[i]->label.rfind("lbrack", 0) == 0) ++i;
    ASTNode* idxTy = nullptr;
    if (i < n->children.size()) {
        if (n->children[i]->label == "<range>") idxTy = buildRange(n->children[i]);
        else if (n->children[i]->label == "<enumerated>") idxTy = buildEnum(n->children[i]);
        else if (n->children[i]->label.rfind("ident(", 0) == 0) idxTy = new TypeRefNode(stripIdent(n->children[i]->label));
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("rbrack", 0) == 0) ++i;
    if (i < n->children.size() && n->children[i]->label.rfind("ofsy", 0) == 0) ++i;
    ASTNode* elTy = nullptr;
    if (i < n->children.size() && n->children[i]->label == "<type>") {
        elTy = buildType(n->children[i]);
    }
    auto* arr = new ArrayTypeNode();
    arr->indexType = idxTy;
    arr->elementType = elTy;
    return arr;
}

static ASTNode* buildRecordType(ParseNode* n) {
    if (!n || n->label != "<record-type>") return nullptr;
    size_t i = 1; 
    auto* rec = new RecordTypeNode();
    if (i < n->children.size() && n->children[i]->label == "<field-list>") {
        ParseNode* fl = n->children[i];
        for (ParseNode* fc : fl->children) {
            if (fc->label == "<field-part>") {
                auto* fd = new FieldDeclNode();
                size_t fi = 0;
                if (fi < fc->children.size() && fc->children[fi]->label == "<identifier-list>") {
                    for (ParseNode* ic : fc->children[fi]->children) {
                        if (ic->label.rfind("ident(", 0) == 0) fd->names.push_back(stripIdent(ic->label));
                    }
                    ++fi;
                }
                if (fi < fc->children.size() && fc->children[fi]->label.rfind("colon", 0) == 0) ++fi;
                if (fi < fc->children.size() && fc->children[fi]->label == "<type>") {
                    fd->typeNode = buildType(fc->children[fi]);
                }
                rec->fields.push_back(fd);
            }
        }
    }
    return rec;
}

static ASTNode* buildConstant(ParseNode* n) {
    if (!n || n->label != "<constant>") return nullptr;
    if (n->children.empty()) return nullptr;
    size_t ci = 0;
    std::string sign;
    if (n->children[ci]->label.rfind("plus", 0) == 0 || n->children[ci]->label.rfind("minus", 0) == 0) {
        sign = n->children[ci]->label.rfind("plus", 0) == 0 ? "+" : "-";
        ++ci;
    }
    if (ci < n->children.size()) {
        std::string lit = n->children[ci]->label;
        if (lit.rfind("intcon(", 0) == 0) {
            return new NumberNode(sign + stripTokenValue(lit), false);
        } else if (lit.rfind("realcon(", 0) == 0) {
            return new NumberNode(sign + stripTokenValue(lit), true);
        } else if (lit.rfind("charcon(", 0) == 0) {
            return new CharNode(stripTokenValue(lit));
        } else if (lit.rfind("string(", 0) == 0) {
            return new StringNode(stripTokenValue(lit));
        } else if (lit.rfind("ident(", 0) == 0) {
            return new VarNode(stripTokenValue(lit));
        }
    }
    return nullptr;
}

static ASTNode* buildRange(ParseNode* n) {
    if (!n || n->label != "<range>") return nullptr;
    auto* r = new RangeNode();

    for (ParseNode* child : n->children) {
        if (child->label == "<constant>") {
            if (!r->low) r->low = buildConstant(child);
            else r->high = buildConstant(child);
        }
    }
    return r;
}

static ASTNode* buildEnum(ParseNode* n) {
    if (!n || n->label != "<enumerated>") return nullptr;
    auto* e = new EnumNode();
    for (ParseNode* child : n->children) {
        if (child->label.rfind("ident(", 0) == 0) {
            e->identifiers.push_back(stripIdent(child->label));
        }
    }
    return e;
}

static ASTNode* buildCompound(ParseNode* n) {
    if (!n || n->label != "<compound-statement>") return nullptr;
    auto* blk = new BlockNode();
    for (ParseNode* child : n->children) {
        if (child->label == "<statement-list>") {
            blk->stmts = buildStmtList(child);
        }
    }
    return blk;
}

static std::vector<ASTNode*> buildStmtList(ParseNode* n) {
    std::vector<ASTNode*> stmts;
    if (!n || n->label != "<statement-list>") return stmts;
    for (ParseNode* child : n->children) {
        if (!child) continue;
        if (child->label == "<statement>") {
            ASTNode* s = buildStmt(child);
            if (s) stmts.push_back(s);
        }

    }
    return stmts;
}

static ASTNode* buildStmt(ParseNode* n) {
    if (!n || n->label != "<statement>") return nullptr;
    if (n->children.empty()) return nullptr; 
    ParseNode* child = n->children[0];
    if (!child) return nullptr;
    if (child->label == "<assignment-statement>") return buildAssign(child);
    if (child->label == "<if-statement>") return buildIf(child);
    if (child->label == "<case-statement>") return buildCase(child);
    if (child->label == "<while-statement>") return buildWhile(child);
    if (child->label == "<repeat-statement>") return buildRepeat(child);
    if (child->label == "<for-statement>") return buildFor(child);
    if (child->label == "<procedure/function-call>") return buildCall(child);
    return nullptr;
}

static ASTNode* buildAssign(ParseNode* n) {
    if (!n || n->label != "<assignment-statement>" || n->children.size() < 3) return nullptr;
    auto* a = new AssignNode();
    a->target = buildVariable(n->children[0]);
    a->value = buildExpr(n->children[2]); 
    return a;
}

static ASTNode* buildIf(ParseNode* n) {
    if (!n || n->label != "<if-statement>") return nullptr;
    size_t i = 1; 
    auto* node = new IfNode();
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->condition = buildExpr(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("thensy", 0) == 0) ++i;
    if (i < n->children.size()) {
        node->thenBranch = buildStmt(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("elsesy", 0) == 0) {
        ++i;
        if (i < n->children.size()) node->elseBranch = buildStmt(n->children[i]);
    }
    return node;
}

static ASTNode* buildCase(ParseNode* n) {
    if (!n || n->label != "<case-statement>") return nullptr;
    size_t i = 1; 
    auto* node = new CaseNode();
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->expr = buildExpr(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("ofsy", 0) == 0) ++i;
    while (i < n->children.size()) {
        if (n->children[i]->label == "<case-block>") {
            node->arms.push_back(buildCaseArm(n->children[i]));
            ++i;
        } else if (n->children[i]->label.rfind("semicolon", 0) == 0) {
            ++i;
        } else if (n->children[i]->label.rfind("endsy", 0) == 0) {
            break;
        } else {
            ++i;
        }
    }
    return node;
}

static ASTNode* buildCaseArm(ParseNode* n) {
    if (!n || n->label != "<case-block>") return nullptr;
    auto* arm = new CaseArmNode();
    size_t i = 0;
    while (i < n->children.size()) {
        if (n->children[i]->label == "<constant>") {
            arm->constants.push_back(buildConstant(n->children[i]));
            ++i;
        } else if (n->children[i]->label.rfind("comma", 0) == 0) {
            ++i;
        } else if (n->children[i]->label.rfind("colon", 0) == 0) {
            ++i;
            if (i < n->children.size()) arm->stmt = buildStmt(n->children[i]);
            ++i;
            break;
        } else {
            ++i;
        }
    }
    return arm;
}

static ASTNode* buildWhile(ParseNode* n) {
    if (!n || n->label != "<while-statement>") return nullptr;
    size_t i = 1; 
    auto* node = new WhileNode();
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->condition = buildExpr(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("dosy", 0) == 0) ++i;
    if (i < n->children.size() && n->children[i]->label == "<compound-statement>") {
        node->body = buildCompound(n->children[i]);
        ++i;
    }
    return node;
}

static ASTNode* buildRepeat(ParseNode* n) {
    if (!n || n->label != "<repeat-statement>") return nullptr;
    size_t i = 1; 
    auto* node = new RepeatNode();
    if (i < n->children.size() && n->children[i]->label == "<statement-list>") {
        node->stmts = buildStmtList(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("untilsy", 0) == 0) ++i;
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->condition = buildExpr(n->children[i]);
    }
    return node;
}

static ASTNode* buildFor(ParseNode* n) {
    if (!n || n->label != "<for-statement>") return nullptr;
    size_t i = 1; 
    std::string varName;
    if (i < n->children.size() && n->children[i]->label.rfind("ident(", 0) == 0) {
        varName = stripIdent(n->children[i]->label);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("becomes", 0) == 0) ++i;
    auto* node = new ForNode(varName);
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->init = buildExpr(n->children[i]);
        ++i;
    }
    if (i < n->children.size()) {
        std::string lbl = n->children[i]->label;
        if (lbl.rfind("tosy", 0) == 0) node->isTo = true;
        else if (lbl.rfind("downtosy", 0) == 0) node->isTo = false;
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label == "<expression>") {
        node->endExpr = buildExpr(n->children[i]);
        ++i;
    }
    if (i < n->children.size() && n->children[i]->label.rfind("dosy", 0) == 0) ++i;
    if (i < n->children.size() && n->children[i]->label == "<compound-statement>") {
        node->body = buildCompound(n->children[i]);
    }
    return node;
}

static ASTNode* buildCall(ParseNode* n) {
    if (!n || n->label != "<procedure/function-call>") return nullptr;
    size_t i = 0;
    std::string name;
    if (i < n->children.size() && n->children[i]->label.rfind("ident(", 0) == 0) {
        name = stripIdent(n->children[i]->label);
        ++i;
    }
    auto* call = new CallNode(name);
    if (i < n->children.size() && n->children[i]->label.rfind("lparent", 0) == 0) {
        ++i;
        if (i < n->children.size() && n->children[i]->label == "<parameter-list>") {
            ParseNode* pl = n->children[i];
            for (ParseNode* c : pl->children) {
                if (c->label == "<expression>") {
                    ASTNode* arg = buildExpr(c);
                    if (arg) call->args.push_back(arg);
                }
            }
            ++i;
        }
    }
    return call;
}

static ASTNode* buildExpr(ParseNode* n) {
    if (!n || n->label != "<expression>") return nullptr;
    if (n->children.size() == 1) {
        return buildSimple(n->children[0]);
    }
    if (n->children.size() == 3) {
        auto* bin = new BinOpNode("");
        bin->left = buildSimple(n->children[0]);

        if (n->children[1] && !n->children[1]->children.empty()) {
            std::string opLabel = n->children[1]->children[0]->label;
            if (opLabel.rfind("eql", 0) == 0) bin->op = "==";
            else if (opLabel.rfind("neq", 0) == 0) bin->op = "<>";
            else if (opLabel.rfind("gtr", 0) == 0) bin->op = ">";
            else if (opLabel.rfind("geq", 0) == 0) bin->op = ">=";
            else if (opLabel.rfind("lss", 0) == 0) bin->op = "<";
            else if (opLabel.rfind("leq", 0) == 0) bin->op = "<=";
        }
        bin->right = buildSimple(n->children[2]);
        return bin;
    }
    return nullptr;
}

static ASTNode* buildSimple(ParseNode* n) {
    if (!n || n->label != "<simple-expression>") return nullptr;
    size_t i = 0;
    std::string unaryOp;
    if (i < n->children.size()) {
        std::string lbl = n->children[i]->label;
        if (lbl.rfind("plus", 0) == 0) { unaryOp = "+"; ++i; }
        else if (lbl.rfind("minus", 0) == 0) { unaryOp = "-"; ++i; }
    }
    ASTNode* term = buildTerm(n->children[i]);
    ++i;
    while (i < n->children.size()) {
        if (n->children[i]->label.rfind("<additive-operator>", 0) == 0) {
            auto* bin = new BinOpNode("");
            std::string opLabel = n->children[i]->children[0]->label;
            if (opLabel.rfind("plus", 0) == 0) bin->op = "+";
            else if (opLabel.rfind("minus", 0) == 0) bin->op = "-";
            else if (opLabel.rfind("orsy", 0) == 0) bin->op = "or";
            bin->left = term;
            ++i;
            if (i < n->children.size()) {
                bin->right = buildTerm(n->children[i]);
                ++i;
            }
            term = bin;
        } else {
            ++i;
        }
    }
    if (!unaryOp.empty() && term) {
        auto* un = new UnaryOpNode(unaryOp);
        un->operand = term;
        return un;
    }
    return term;
}

static ASTNode* buildTerm(ParseNode* n) {
    if (!n || n->label != "<term>") return nullptr;
    if (n->children.empty()) return nullptr;
    ASTNode* factor = buildFactor(n->children[0]);
    size_t i = 1;
    while (i < n->children.size()) {
        if (n->children[i]->label.rfind("<multiplicative-operator>", 0) == 0) {
            auto* bin = new BinOpNode("");
            std::string opLabel = n->children[i]->children[0]->label;
            if (opLabel.rfind("times", 0) == 0) bin->op = "*";
            else if (opLabel.rfind("rdiv", 0) == 0) bin->op = "/";
            else if (opLabel.rfind("idiv", 0) == 0) bin->op = "div";
            else if (opLabel.rfind("imod", 0) == 0) bin->op = "mod";
            else if (opLabel.rfind("andsy", 0) == 0) bin->op = "and";
            bin->left = factor;
            ++i;
            if (i < n->children.size()) {
                bin->right = buildFactor(n->children[i]);
                ++i;
            }
            factor = bin;
        } else {
            ++i;
        }
    }
    return factor;
}

static ASTNode* buildFactor(ParseNode* n) {
    if (!n || n->label != "<factor>") return nullptr;
    if (n->children.empty()) return nullptr;
    ParseNode* child = n->children[0];
    std::string lbl = child->label;
    if (lbl.rfind("intcon(", 0) == 0) {
        return new NumberNode(stripTokenValue(lbl), false);
    }
    if (lbl.rfind("realcon(", 0) == 0) {
        return new NumberNode(stripTokenValue(lbl), true);
    }
    if (lbl.rfind("charcon(", 0) == 0) {
        return new CharNode(stripTokenValue(lbl));
    }
    if (lbl.rfind("string(", 0) == 0) {
        return new StringNode(stripTokenValue(lbl));
    }
    if (lbl.rfind("notsy", 0) == 0) {
        auto* un = new UnaryOpNode("not");
        if (n->children.size() > 1) un->operand = buildFactor(n->children[1]);
        return un;
    }
    if (lbl.rfind("lparent", 0) == 0) {
        if (n->children.size() > 1 && n->children[1]->label == "<expression>") {
            return buildExpr(n->children[1]);
        }
        return nullptr;
    }
    if (lbl == "<procedure/function-call>") {
        return buildCall(child);
    }
    if (lbl == "<variable>") {
        return buildVariable(child);
    }
    return nullptr;
}

static ASTNode* buildVariable(ParseNode* n) {
    if (!n || n->label != "<variable>") return nullptr;
    if (n->children.empty()) return nullptr;
    ASTNode* base = nullptr;
    if (n->children[0]->label.rfind("ident(", 0) == 0) {
        base = new VarNode(stripIdent(n->children[0]->label));
    }
    size_t i = 1;
    while (i < n->children.size()) {
        ParseNode* cv = n->children[i];
        if (cv->label == "<component-variable>") {
            if (cv->children.size() >= 2) {
                std::string first = cv->children[0]->label;
                if (first.rfind("lbrack", 0) == 0) {

                    auto* idx = new IndexNode(base);
                    if (cv->children.size() >= 2 && cv->children[1]->label == "<index-list>") {
                        for (ParseNode* ic : cv->children[1]->children) {
                            std::string il = ic->label;
                            if (il.rfind("intcon(", 0) == 0) idx->indices.push_back(new NumberNode(stripTokenValue(il), false));
                            else if (il.rfind("charcon(", 0) == 0) idx->indices.push_back(new CharNode(stripTokenValue(il)));
                            else if (il.rfind("ident(", 0) == 0) idx->indices.push_back(new VarNode(stripTokenValue(il)));
                        }
                    }
                    base = idx;
                } else if (first.rfind("period", 0) == 0) {

                    std::string fieldName;
                    if (cv->children.size() >= 2 && cv->children[1]->label.rfind("ident(", 0) == 0) {
                        fieldName = stripIdent(cv->children[1]->label);
                    }
                    base = new FieldAccessNode(base, fieldName);
                }
            }
        }
        ++i;
    }
    return base;
}

void printAST(ASTNode* root, std::ostream& os) {
    if (root) root->print(os, "", true, true);
}

void writeAST(ASTNode* root, const std::string& outputPath) {
    if (!root) return;
    namespace fs = std::filesystem;
    fs::path outDir = fs::path(outputPath).parent_path();
    if (!outDir.empty() && !fs::exists(outDir))
        fs::create_directories(outDir);
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        throw std::runtime_error("cannot open output file: " + outputPath);
    }
    printAST(root, out);
}