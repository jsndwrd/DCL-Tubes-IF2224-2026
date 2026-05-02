#ifndef PARSER_HPP
#define PARSER_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

struct ParseNode {
    std::string label;
    std::vector<ParseNode*> children;
};

class SyntaxError : public std::runtime_error {
public:
    explicit SyntaxError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    ParseNode* parse();

private:
    std::vector<Token> tokens;
    int pos;

    Token peek() const;
    Token peekAt(int offset) const;
    Token consume();
    void expect(ArionToken expectedType);

    ParseNode* parseProgram();
    ParseNode* parseHeader();
    ParseNode* parseDeclaration();
    ParseNode* parseConsts();
    ParseNode* parseConstDef();
    ParseNode* parseTypes();
    ParseNode* parseVars();
    ParseNode* parseIdList();
    ParseNode* parseType();
    ParseNode* parseArray();
    ParseNode* parseRange();
    ParseNode* parseEnum();
    ParseNode* parseRecord();
    ParseNode* parseFields();
    ParseNode* parseField();
    ParseNode* parseSub();
    ParseNode* parseProc();
    ParseNode* parseFunc();
    ParseNode* parseBlock();
    ParseNode* parseFormals();
    ParseNode* parseParamGroup();
    ParseNode* parseCompound();
    ParseNode* parseStmts();
    ParseNode* parseStmt();
    ParseNode* parseVar();
    ParseNode* parseCompVar();
    ParseNode* parseIndices();
    ParseNode* parseAssign();
    ParseNode* parseIf();
    ParseNode* parseCase();
    ParseNode* parseCaseArm();
    ParseNode* parseWhile();
    ParseNode* parseRepeat();
    ParseNode* parseFor();
    ParseNode* parseCall();
    ParseNode* parseArgs();
    ParseNode* parseExpr();
    ParseNode* parseSimple();
    ParseNode* parseTerm();
    ParseNode* parseFactor();
    ParseNode* parseRelOp();
    ParseNode* parseAddOp();
    ParseNode* parseMulOp();
};

#endif
