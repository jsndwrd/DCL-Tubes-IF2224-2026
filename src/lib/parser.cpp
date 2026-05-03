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
    auto* n = new ParseNode{"<const-declaration>", {}};
    n->children.push_back(new ParseNode{"constsy", {}});
    expect(CONSTSY);
    do {
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(IDENT);
        expect(EQL);
        n->children.push_back(parseConstDef());
        expect(SEMICOLON);
    } while (peek().type == IDENT && peekAt(1).type == EQL);
    return n;
}

ParseNode* Parser::parseConstDef() {
    auto* n = new ParseNode{"<constant>", {}};
    Token t = peek();
    checkNotUnknown(t);

    if (t.type == CHARCON || t.type == STRING) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        return n;
    }

    if (t.type == PLUS || t.type == MINUS) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
    }

    Token u = peek();
    checkNotUnknown(u);
    if (u.type == IDENT || u.type == INTCON || u.type == REALCON) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        return n;
    }

    throw SyntaxError("Syntax error at line " + std::to_string(u.line) + ": unexpected token " +
                      u.toString() + ", expected ident, intcon, or realcon in <constant>");
}

ParseNode* Parser::parseTypes() {
    auto* n = new ParseNode{"<type-declaration>", {}};
    n->children.push_back(new ParseNode{"typesy", {}});
    expect(TYPESY);
    do {
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(IDENT);
        expect(EQL);
        n->children.push_back(parseType());
        expect(SEMICOLON);
    } while (peek().type == IDENT && peekAt(1).type == EQL);
    return n;
}

ParseNode* Parser::parseVars() {
    auto* n = new ParseNode{"<var-declaration>", {}};
    n->children.push_back(new ParseNode{"varsy", {}});
    expect(VARSY);
    do {
        n->children.push_back(parseIdList());
        expect(COLON);
        n->children.push_back(parseType());
        expect(SEMICOLON);
    } while (peek().type == IDENT);
    return n;
}

ParseNode* Parser::parseIdList() {
    auto* n = new ParseNode{"<identifier-list>", {}};
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(IDENT);
    }
    return n;
}

ParseNode* Parser::parseType() {
    auto* n = new ParseNode{"<type>", {}};
    Token t = peek();
    checkNotUnknown(t);

    if (t.type == ARRAYSY) {
        n->children.push_back(parseArray());
        return n;
    }
    if (t.type == RECORDSY) {
        n->children.push_back(parseRecord());
        return n;
    }
    if (t.type == LPARENT) {
        n->children.push_back(parseEnum());
        return n;
    }
    if (t.type == IDENT) {
        if (peekAt(1).type == PERIOD && peekAt(2).type == PERIOD) {
            n->children.push_back(parseRange());
            return n;
        }
        n->children.push_back(new ParseNode{consume().toString(), {}});
        return n;
    }
    if (canStartConstant(t.type)) {
        n->children.push_back(parseRange());
        return n;
    }

    throw SyntaxError("Syntax error at line " + std::to_string(t.line) + ": unexpected token " +
                      t.toString() + ", expected <type>");
}

ParseNode* Parser::parseArray() {
    auto* n = new ParseNode{"<array-type>", {}};
    expect(ARRAYSY);
    expect(LBRACK);
    Token a = peek();
    checkNotUnknown(a);
    if (a.type == IDENT) {
        if (peekAt(1).type == PERIOD && peekAt(2).type == PERIOD) {
            n->children.push_back(parseRange());
        } else {
            n->children.push_back(new ParseNode{consume().toString(), {}});
        }
    } else if (canStartConstant(a.type)) {
        n->children.push_back(parseRange());
    } else {
        throw SyntaxError("Syntax error at line " + std::to_string(a.line) +
                          ": unexpected token " + a.toString() + " in <array-type> bound");
    }
    expect(RBRACK);
    expect(OFSY);
    n->children.push_back(parseType());
    return n;
}

ParseNode* Parser::parseRange() {
    auto* n = new ParseNode{"<range>", {}};
    n->children.push_back(parseConstDef());
    expect(PERIOD);
    expect(PERIOD);
    n->children.push_back(parseConstDef());
    return n;
}

ParseNode* Parser::parseEnum() {
    auto* n = new ParseNode{"<enumerated>", {}};
    expect(LPARENT);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(IDENT);
    }
    expect(RPARENT);
    return n;
}

ParseNode* Parser::parseRecord() {
    auto* n = new ParseNode{"<record-type>", {}};
    expect(RECORDSY);
    n->children.push_back(parseFields());
    expect(ENDSY);
    return n;
}

ParseNode* Parser::parseFields() {
    auto* n = new ParseNode{"<field-list>", {}};
    while (peek().type != ENDSY) {
        n->children.push_back(parseField());
        if (peek().type == ENDSY) break;
        expect(SEMICOLON);
    }
    return n;
}

ParseNode* Parser::parseField() {
    auto* n = new ParseNode{"<field-part>", {}};
    n->children.push_back(parseIdList());
    expect(COLON);
    n->children.push_back(parseType());
    return n;
}

ParseNode* Parser::parseSub() {
    auto* n = new ParseNode{"<subprogram-declaration>", {}};
    if (peek().type == PROCEDURESY) {
        n->children.push_back(parseProc());
    } else if (peek().type == FUNCTIONSY) {
        n->children.push_back(parseFunc());
    } else {
        Token t = peek();
        checkNotUnknown(t);
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() +
                          ", expected proceduresy or functionsy");
    }
    return n;
}

ParseNode* Parser::parseProc() {
    auto* n = new ParseNode{"<procedure-declaration>", {}};
    expect(PROCEDURESY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    if (peek().type == LPARENT) {
        n->children.push_back(parseFormals());
    }
    expect(SEMICOLON);
    n->children.push_back(parseBlock());
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseFunc() {
    auto* n = new ParseNode{"<function-declaration>", {}};
    expect(FUNCTIONSY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    if (peek().type == LPARENT) {
        n->children.push_back(parseFormals());
    }
    expect(COLON);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    expect(SEMICOLON);
    n->children.push_back(parseBlock());
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseBlock() {
    auto* n = new ParseNode{"<block>", {}};
    n->children.push_back(parseDeclaration());
    n->children.push_back(parseCompound());
    return n;
}

ParseNode* Parser::parseFormals() {
    auto* n = new ParseNode{"<formal-parameter-list>", {}};
    expect(LPARENT);
    n->children.push_back(parseParamGroup());
    while (peek().type == SEMICOLON) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseParamGroup());
    }
    expect(RPARENT);
    return n;
}

ParseNode* Parser::parseParamGroup() {
    auto* n = new ParseNode{"<parameter-group>", {}};
    n->children.push_back(parseIdList());
    expect(COLON);
    Token t = peek();
    checkNotUnknown(t);
    if (t.type == ARRAYSY) {
        n->children.push_back(parseArray());
    } else if (t.type == IDENT) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
    } else {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() +
                          ", expected ident or arraysy in <parameter-group>");
    }
    return n;
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

