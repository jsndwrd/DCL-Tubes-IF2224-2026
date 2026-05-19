// Header symbol table (tab, btab, atab) dan semantic analyzer
#ifndef SYMTAB_HPP
#define SYMTAB_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include "ast.hpp"

enum ObjClass {
    OBJ_CONSTANT  = 1,
    OBJ_VARIABLE  = 2,
    OBJ_TYPE      = 3,
    OBJ_PROCEDURE = 4,
    OBJ_FUNCTION  = 5,
    OBJ_PROGRAM   = 6,
    OBJ_FIELD     = 7
};

struct TabEntry {
    std::string id;   
    int link;         
    int obj;          
    int type;         
    int ref;          
    int nrm;          
    int lev;          
    int adr;          
};

struct BTabEntry {
    int last;  
    int lpar;  
    int psze;  
    int vsze;  
};

struct ATabEntry {
    int xtyp;  
    int etyp;  
    int eref;  
    int low;   
    int high;  
    int elsz;  
    int size;  
};

class SemanticError : public std::runtime_error {
public:
    explicit SemanticError(const std::string& msg) : std::runtime_error(msg) {}
};

class SymbolTable {
public:
    SymbolTable();

    void enterBlock(bool isProcOrFunc = false);
    void exitBlock();
    int currentLevel() const { return level; }
    int currentBTab() const { return display.empty() ? 0 : display.back(); }

    int enter(const std::string& id, int obj, int type, int ref = 0, int nrm = 1, int adr = 0);
    int enterParameter(const std::string& id, int type, int ref = 0, bool isVar = false);

    int lookup(const std::string& id) const;
    int lookupInCurrentBlock(const std::string& id) const;

    int findType(const std::string& name) const; 
    bool isSimpleType(int type) const;
    bool isNumericType(int type) const;
    bool compatible(int t1, int t2) const;
    bool assignmentCompatible(int targetType, int targetRef, int exprType, int exprRef) const;

    void printTab(std::ostream& os = std::cout) const;
    void printBTab(std::ostream& os = std::cout) const;
    void printATab(std::ostream& os = std::cout) const;

    std::vector<TabEntry> tab;
    std::vector<BTabEntry> btab;
    std::vector<ATabEntry> atab;
    std::vector<int> display;
    int level;

    int dx; 
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    void analyze(ASTNode* root);
    SymbolTable sym;

private:
    void visit(ASTNode* node);
    void visitProgram(ProgramNode* n);
    void visitBlock(BlockNode* n, const std::vector<ASTNode*>& extraDecls = {});
    void visitVarDecl(VarDeclNode* n);
    void visitConstDecl(ConstDeclNode* n);
    void visitTypeDecl(TypeDeclNode* n);
    void visitProcDecl(ProcDeclNode* n);
    void visitFuncDecl(FuncDeclNode* n);
    void visitFieldDecl(FieldDeclNode* n, int recordBTab);
    void visitParam(ParamNode* n);

    void visitStmt(ASTNode* n);
    void visitAssign(AssignNode* n);
    void visitIf(IfNode* n);
    void visitWhile(WhileNode* n);
    void visitFor(ForNode* n);
    void visitRepeat(RepeatNode* n);
    void visitCase(CaseNode* n);
    void visitCall(CallNode* n);

    DataType visitExpr(ASTNode* n);
    DataType visitBinOp(BinOpNode* n);
    DataType visitUnaryOp(UnaryOpNode* n);
    DataType visitVar(VarNode* n);
    DataType visitIndex(IndexNode* n);
    DataType visitFieldAccess(FieldAccessNode* n);
    DataType visitNumber(NumberNode* n);
    DataType visitString(StringNode* n);
    DataType visitChar(CharNode* n);
    DataType visitBool(BoolNode* n);

    DataType visitTypeNode(ASTNode* n, int& outRef);
    DataType visitRangeNode(RangeNode* n, int& outRef);
    DataType visitEnumNode(EnumNode* n, int& outRef);
    DataType visitArrayType(ArrayTypeNode* n, int& outRef);
    DataType visitRecordType(RecordTypeNode* n, int& outRef);

    void expectType(DataType got, DataType expected, const std::string& ctx);
    void expectNumeric(DataType got, const std::string& ctx);
    void expectBoolean(DataType got, const std::string& ctx);
};

#endif