#include "../header/parser.hpp"

#include <string>
#include <utility>

namespace {

Token eofToken() {
    return Token(EOFILE, "", 0);
}

const char* tokenType(ArionToken type) {
    if (type < 0 || type > UNKNOWN) return "undefined";
    return ArionTokenName[type];
}

}

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), pos(0) {}

Token Parser::peek() const {
    if (pos < 0 || pos >= static_cast<int>(this->tokens.size())) return eofToken();
    return this->tokens[static_cast<std::size_t>(pos)];
}

Token Parser::peekAt(int offset) const {
    const long long i = static_cast<long long>(pos) + static_cast<long long>(offset);
    if (i < 0 || i >= static_cast<long long>(this->tokens.size())) return eofToken();
    return this->tokens[static_cast<std::size_t>(i)];
}

Token Parser::consume() {
    if (pos < 0 || pos >= static_cast<int>(this->tokens.size())) return eofToken();
    Token t = this->tokens[static_cast<std::size_t>(pos)];
    ++pos;
    return t;
}

void Parser::expect(ArionToken expectedType) {
    Token t = peek();
    if (t.type != expectedType) {
        throw SyntaxError(
            "Syntax error at line " + std::to_string(t.line) + ": unexpected token " +
            t.toString() + ", expected " + tokenType(expectedType));
        }
        consume();
}

ParseNode* Parser::parse() {

}
    
ParseNode* Parser::parseProgram() {

}

ParseNode* Parser::parseHeader() {

}

ParseNode* Parser::parseDeclaration() {

}

ParseNode* Parser::parseConsts() {

}

ParseNode* Parser::parseConstDef() {

}

ParseNode* Parser::parseTypes() {

}

ParseNode* Parser::parseVars() {

}

ParseNode* Parser::parseIdList() {

}

ParseNode* Parser::parseType() {

}

ParseNode* Parser::parseArray() {

}

ParseNode* Parser::parseRange() {

}

ParseNode* Parser::parseEnum() {

}

ParseNode* Parser::parseRecord() {

}

ParseNode* Parser::parseFields() {

}

ParseNode* Parser::parseField() {

}

ParseNode* Parser::parseSub() {

}

ParseNode* Parser::parseProc() {

}

ParseNode* Parser::parseFunc() {

}

ParseNode* Parser::parseBlock() {

}

ParseNode* Parser::parseFormals() {

}

ParseNode* Parser::parseParamGroup() {

}

ParseNode* Parser::parseCompound() {

}

ParseNode* Parser::parseStmts() {

}

ParseNode* Parser::parseStmt() {

}

ParseNode* Parser::parseVar() {

}

ParseNode* Parser::parseCompVar() {

}

ParseNode* Parser::parseIndices() {

}

ParseNode* Parser::parseAssign() {

}

ParseNode* Parser::parseIf() {

}

ParseNode* Parser::parseCase() {

}

ParseNode* Parser::parseCaseArm() {

}

ParseNode* Parser::parseWhile() {

}

ParseNode* Parser::parseRepeat() {

}

ParseNode* Parser::parseFor() {

}

ParseNode* Parser::parseCall() {

}

ParseNode* Parser::parseArgs() {

}

ParseNode* Parser::parseExpr() {

}

ParseNode* Parser::parseSimple() {

}

ParseNode* Parser::parseTerm() {

}

ParseNode* Parser::parseFactor() {

}

ParseNode* Parser::parseRelOp() {

}

ParseNode* Parser::parseAddOp() {

}

ParseNode* Parser::parseMulOp() {

}

