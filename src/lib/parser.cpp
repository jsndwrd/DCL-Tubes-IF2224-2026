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

bool isRelational(ArionToken t) {
    return t == EQL || t == NEQ || t == GTR || t == GEQ || t == LSS || t == LEQ;
}

bool isAdditiveOp(ArionToken t) {
    return t == PLUS || t == MINUS || t == ORSY;
}

bool isMultiplicativeOp(ArionToken t) {
    return t == TIMES || t == RDIV || t == IDIV || t == IMOD || t == ANDSY;
}

bool canStartConstant(ArionToken t) {
    return t == PLUS || t == MINUS || t == IDENT || t == INTCON || t == REALCON || t == CHARCON ||
            t == STRING;
}

void checkNotUnknown(const Token& t) {
    if (t.type == UNKNOWN) {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                            ": unexpected token " + t.toString());
    }
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
    ParseNode* root = parseProgram();
    Token t = peek();
    if (t.type != EOFILE) {
        checkNotUnknown(t);
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected eofile");
    }
    return root;
}

ParseNode* Parser::parseProgram() {
    auto* n = new ParseNode{"<program>", {}};
    n->children.push_back(parseHeader());
    n->children.push_back(parseDeclaration());
    n->children.push_back(parseCompound());
    n->children.push_back(new ParseNode{"period", {}});
    expect(PERIOD);
    return n;
}

ParseNode* Parser::parseHeader() {
    auto* n = new ParseNode{"<program-header>", {}};
    n->children.push_back(new ParseNode{"programsy", {}});
    expect(PROGRAMSY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    n->children.push_back(new ParseNode{"semicolon", {}});
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseDeclaration() {
    auto* n = new ParseNode{"<declaration-part>", {}};

    while (peek().type == CONSTSY) {
        n->children.push_back(parseConsts());
    }

    while (peek().type == TYPESY) {
        n->children.push_back(parseTypes());
    }

    while (peek().type == VARSY) {
        n->children.push_back(parseVars());
    }

    while (peek().type == PROCEDURESY || peek().type == FUNCTIONSY) {
        n->children.push_back(parseSub());
    }

    Token nx = peek();
    if (nx.type != BEGINSY) {
        checkNotUnknown(nx);
        throw SyntaxError("Syntax error at line " + std::to_string(nx.line) +
                          ": unexpected token " + nx.toString() + ", expected beginsy (strict "
                          "declaration order: const, type, var, subprogram)");
    }

    return n;
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

