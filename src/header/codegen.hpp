#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "ast.hpp"
#include "symtab.hpp"
#include "instruction.hpp"

constexpr int FRAME_HEADER_SIZE = 3;

class CodeGenError : public std::runtime_error {
public:
    explicit CodeGenError(const std::string& msg) : std::runtime_error(msg) {}
};

class CodeGenerator {
public:
    explicit CodeGenerator(SymbolTable& sym);

    std::vector<Instruction> generate(ASTNode* root);

    const std::vector<Instruction>& code() const { return instr; }

private:
    SymbolTable& sym;
    std::vector<Instruction> instr;
    std::unordered_map<int, int> procEntry;
    std::vector<int> pendingCalls;
    int curLevel;

    void resolveAddresses();
    void resolveBlockAddresses(int btabIdx);

    int emit(OpCode op, int level, int operand);
    int nextLine() const { return static_cast<int>(instr.size()); }
    void patch(int instrIdx, int operand);

    void genProgram(ProgramNode* n);
    void genBlock(BlockNode* n);
    void genStmt(ASTNode* n);
    void genAssign(AssignNode* n);
    void genIf(IfNode* n);
    void genWhile(WhileNode* n);
    void genFor(ForNode* n);
    void genRepeat(RepeatNode* n);
    void genCall(CallNode* n);

    void genExpr(ASTNode* n);
    void genBinOp(BinOpNode* n);
    void genUnaryOp(UnaryOpNode* n);
    void genVar(VarNode* n);
    void genNumber(NumberNode* n);
    void genBool(BoolNode* n);
    void genChar(CharNode* n);

    void genSubprogram(ASTNode* n);

    int levelDiff(int useLevel, int declLevel) const;
    OprCode binOpToOpr(const std::string& op) const;
};

#endif
