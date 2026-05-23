// Header untuk struktur AST dan builder-nya
#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <iostream>

enum DataType {
    T_ERROR    = -1,
    T_VOID     = 0,
    T_INTEGER  = 1,
    T_REAL     = 2,
    T_CHAR     = 3,
    T_BOOLEAN  = 4,
    T_STRING   = 5,
    T_SUBRANGE = 6,
    T_ENUM     = 7,
    T_ARRAY    = 8,
    T_RECORD   = 9
};

const char* dataTypeName(DataType dt);

enum ASTNodeType {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VARDECL,
    AST_CONSTDECL,
    AST_TYPEDECL,
    AST_PROCDECL,
    AST_FUNCDECL,
    AST_FIELDDECL,
    AST_PARAM,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_REPEAT,
    AST_CASE,
    AST_CASEARM,
    AST_BINOP,
    AST_UNARYOP,
    AST_CALL,
    AST_VAR,
    AST_INDEX,
    AST_FIELDACCESS,
    AST_NUMBER,
    AST_STRING,
    AST_CHAR,
    AST_BOOL,
    AST_RANGE,
    AST_ENUM,
    AST_ARRAYTYPE,
    AST_RECORDTYPE,
    AST_TYPEREF
};

const char* astNodeTypeName(ASTNodeType nt);

struct ASTNode {
    ASTNodeType nodeType;
    DataType    dataType;
    int         tabIndex;   
    int         lexLevel;   
    int         line;       

    ASTNode(ASTNodeType nt, int line = 0)
        : nodeType(nt), dataType(T_VOID), tabIndex(-1), lexLevel(0), line(line) {}
    virtual ~ASTNode() = default;

    virtual void print(std::ostream& os, const std::string& prefix = "", bool isLast = true, bool isRoot = true) const;
    virtual std::string label() const;
};

void destroyAST(ASTNode* root);

struct ProgramNode : ASTNode {
    std::string name;
    std::vector<ASTNode*> decls; 
    ASTNode* block;              

    ProgramNode(const std::string& name, int line = 0)
        : ASTNode(AST_PROGRAM, line), name(name), block(nullptr) {}
    ~ProgramNode() override {
        for (auto* d : decls) destroyAST(d);
        destroyAST(block);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct BlockNode : ASTNode {
    std::vector<ASTNode*> stmts;
    int blockIndex; 

    BlockNode(int line = 0)
        : ASTNode(AST_BLOCK, line), blockIndex(-1) {}
    ~BlockNode() override {
        for (auto* s : stmts) destroyAST(s);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct VarDeclNode : ASTNode {
    std::vector<std::string> names;
    ASTNode* typeNode; 

    VarDeclNode(int line = 0)
        : ASTNode(AST_VARDECL, line), typeNode(nullptr) {}
    ~VarDeclNode() override { destroyAST(typeNode); }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct ConstDeclNode : ASTNode {
    std::string name;
    ASTNode* value; 

    ConstDeclNode(const std::string& name, int line = 0)
        : ASTNode(AST_CONSTDECL, line), name(name), value(nullptr) {}
    ~ConstDeclNode() override { destroyAST(value); }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct TypeDeclNode : ASTNode {
    std::string name;
    ASTNode* typeNode;

    TypeDeclNode(const std::string& name, int line = 0)
        : ASTNode(AST_TYPEDECL, line), name(name), typeNode(nullptr) {}
    ~TypeDeclNode() override { destroyAST(typeNode); }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct ParamNode : ASTNode {
    std::string name;
    ASTNode* typeNode;
    bool isVar; 

    ParamNode(const std::string& name, int line = 0)
        : ASTNode(AST_PARAM, line), name(name), typeNode(nullptr), isVar(false) {}
    ~ParamNode() override { destroyAST(typeNode); }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct ProcDeclNode : ASTNode {
    std::string name;
    std::vector<ParamNode*> params;
    ASTNode* block; 
    ASTNode* localDecls; 

    std::vector<ASTNode*> decls; 

    ProcDeclNode(const std::string& name, int line = 0)
        : ASTNode(AST_PROCDECL, line), name(name), block(nullptr) {}
    ~ProcDeclNode() override {
        for (auto* p : params) destroyAST(p);
        for (auto* d : decls) destroyAST(d);
        destroyAST(block);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct FuncDeclNode : ASTNode {
    std::string name;
    std::vector<ParamNode*> params;
    ASTNode* returnType; 
    ASTNode* block;
    std::vector<ASTNode*> decls;

    FuncDeclNode(const std::string& name, int line = 0)
        : ASTNode(AST_FUNCDECL, line), name(name), returnType(nullptr), block(nullptr) {}
    ~FuncDeclNode() override {
        for (auto* p : params) destroyAST(p);
        for (auto* d : decls) destroyAST(d);
        destroyAST(returnType);
        destroyAST(block);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct FieldDeclNode : ASTNode {
    std::vector<std::string> names;
    ASTNode* typeNode;

    FieldDeclNode(int line = 0)
        : ASTNode(AST_FIELDDECL, line), typeNode(nullptr) {}
    ~FieldDeclNode() override { destroyAST(typeNode); }
    std::string label() const override;
};

struct AssignNode : ASTNode {
    ASTNode* target; 
    ASTNode* value;

    AssignNode(int line = 0)
        : ASTNode(AST_ASSIGN, line), target(nullptr), value(nullptr) {}
    ~AssignNode() override {
        destroyAST(target);
        destroyAST(value);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct IfNode : ASTNode {
    ASTNode* condition;
    ASTNode* thenBranch;
    ASTNode* elseBranch; 

    IfNode(int line = 0)
        : ASTNode(AST_IF, line), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr) {}
    ~IfNode() override {
        destroyAST(condition);
        destroyAST(thenBranch);
        destroyAST(elseBranch);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct WhileNode : ASTNode {
    ASTNode* condition;
    ASTNode* body; 

    WhileNode(int line = 0)
        : ASTNode(AST_WHILE, line), condition(nullptr), body(nullptr) {}
    ~WhileNode() override {
        destroyAST(condition);
        destroyAST(body);
    }
    std::string label() const override;
};

struct ForNode : ASTNode {
    std::string varName;
    ASTNode* init;
    bool isTo; 
    ASTNode* endExpr;
    ASTNode* body; 

    ForNode(const std::string& varName, int line = 0)
        : ASTNode(AST_FOR, line), varName(varName), init(nullptr), isTo(true), endExpr(nullptr), body(nullptr) {}
    ~ForNode() override {
        destroyAST(init);
        destroyAST(endExpr);
        destroyAST(body);
    }
    std::string label() const override;
};

struct RepeatNode : ASTNode {
    std::vector<ASTNode*> stmts;
    ASTNode* condition;

    RepeatNode(int line = 0)
        : ASTNode(AST_REPEAT, line), condition(nullptr) {}
    ~RepeatNode() override {
        for (auto* s : stmts) destroyAST(s);
        destroyAST(condition);
    }
    std::string label() const override;
};

struct CaseNode : ASTNode {
    ASTNode* expr;
    std::vector<ASTNode*> arms; 

    CaseNode(int line = 0)
        : ASTNode(AST_CASE, line), expr(nullptr) {}
    ~CaseNode() override {
        destroyAST(expr);
        for (auto* a : arms) destroyAST(a);
    }
    std::string label() const override;
};

struct CaseArmNode : ASTNode {
    std::vector<ASTNode*> constants; 
    ASTNode* stmt;

    CaseArmNode(int line = 0)
        : ASTNode(AST_CASEARM, line), stmt(nullptr) {}
    ~CaseArmNode() override {
        for (auto* c : constants) destroyAST(c);
        destroyAST(stmt);
    }
    std::string label() const override;
};

struct BinOpNode : ASTNode {
    std::string op; 
    ASTNode* left;
    ASTNode* right;

    BinOpNode(const std::string& op, int line = 0)
        : ASTNode(AST_BINOP, line), op(op), left(nullptr), right(nullptr) {}
    ~BinOpNode() override {
        destroyAST(left);
        destroyAST(right);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct UnaryOpNode : ASTNode {
    std::string op; 
    ASTNode* operand;

    UnaryOpNode(const std::string& op, int line = 0)
        : ASTNode(AST_UNARYOP, line), op(op), operand(nullptr) {}
    ~UnaryOpNode() override { destroyAST(operand); }
    std::string label() const override;
};

struct CallNode : ASTNode {
    std::string name;
    std::vector<ASTNode*> args;

    CallNode(const std::string& name, int line = 0)
        : ASTNode(AST_CALL, line), name(name) {}
    ~CallNode() override {
        for (auto* a : args) destroyAST(a);
    }
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct VarNode : ASTNode {
    std::string name;

    VarNode(const std::string& name, int line = 0)
        : ASTNode(AST_VAR, line), name(name) {}
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct IndexNode : ASTNode {
    ASTNode* base; 
    std::vector<ASTNode*> indices;

    IndexNode(ASTNode* base, int line = 0)
        : ASTNode(AST_INDEX, line), base(base) {}
    ~IndexNode() override {
        destroyAST(base);
        for (auto* i : indices) destroyAST(i);
    }
    std::string label() const override;
};

struct FieldAccessNode : ASTNode {
    ASTNode* base;
    std::string field;

    FieldAccessNode(ASTNode* base, const std::string& field, int line = 0)
        : ASTNode(AST_FIELDACCESS, line), base(base), field(field) {}
    ~FieldAccessNode() override { destroyAST(base); }
    std::string label() const override;
};

struct NumberNode : ASTNode {
    std::string value; 
    bool isReal;

    NumberNode(const std::string& value, bool isReal, int line = 0)
        : ASTNode(AST_NUMBER, line), value(value), isReal(isReal) {}
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct StringNode : ASTNode {
    std::string value;

    StringNode(const std::string& value, int line = 0)
        : ASTNode(AST_STRING, line), value(value) {}
    std::string label() const override;
    void print(std::ostream& os, const std::string& prefix, bool isLast, bool isRoot) const override;
};

struct CharNode : ASTNode {
    std::string value; 

    CharNode(const std::string& value, int line = 0)
        : ASTNode(AST_CHAR, line), value(value) {}
    std::string label() const override;
};

struct BoolNode : ASTNode {
    bool value;

    BoolNode(bool value, int line = 0)
        : ASTNode(AST_BOOL, line), value(value) {}
    std::string label() const override;
};

struct RangeNode : ASTNode {
    ASTNode* low;
    ASTNode* high;

    RangeNode(int line = 0)
        : ASTNode(AST_RANGE, line), low(nullptr), high(nullptr) {}
    ~RangeNode() override {
        destroyAST(low);
        destroyAST(high);
    }
    std::string label() const override;
};

struct EnumNode : ASTNode {
    std::vector<std::string> identifiers;

    EnumNode(int line = 0)
        : ASTNode(AST_ENUM, line) {}
    std::string label() const override;
};

struct ArrayTypeNode : ASTNode {
    ASTNode* indexType; 
    ASTNode* elementType;

    ArrayTypeNode(int line = 0)
        : ASTNode(AST_ARRAYTYPE, line), indexType(nullptr), elementType(nullptr) {}
    ~ArrayTypeNode() override {
        destroyAST(indexType);
        destroyAST(elementType);
    }
    std::string label() const override;
};

struct RecordTypeNode : ASTNode {
    std::vector<FieldDeclNode*> fields;

    RecordTypeNode(int line = 0)
        : ASTNode(AST_RECORDTYPE, line) {}
    ~RecordTypeNode() override {
        for (auto* f : fields) destroyAST(f);
    }
    std::string label() const override;
};

struct TypeRefNode : ASTNode {
    std::string name;

    TypeRefNode(const std::string& name, int line = 0)
        : ASTNode(AST_TYPEREF, line), name(name) {}
    std::string label() const override;
};

struct ParseNode;

ASTNode* buildAST(ParseNode* root);

void printAST(ASTNode* root, std::ostream& os = std::cout);
void writeAST(ASTNode* root, const std::string& outputPath);

#endif