#include "../header/parser.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
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

bool isAssignFollowed(const std::vector<Token>& tokens, int start) {
    int i = start;
    if (i < 0 || i >= static_cast<int>(tokens.size()) || tokens[static_cast<std::size_t>(i)].type != IDENT) {
        return false;
    }
    ++i;
    while (i < static_cast<int>(tokens.size())) {
        ArionToken ty = tokens[static_cast<std::size_t>(i)].type;
        if (ty == LBRACK) {
            int depth = 1;
            ++i;
            while (i < static_cast<int>(tokens.size()) && depth > 0) {
                ArionToken u = tokens[static_cast<std::size_t>(i)].type;
                if (u == LBRACK) ++depth;
                else if (u == RBRACK)
                    --depth;
                ++i;
            }
            if (depth != 0) return false;
            continue;
        }
        if (ty == PERIOD) {
            ++i;
            if (i >= static_cast<int>(tokens.size()) ||
                tokens[static_cast<std::size_t>(i)].type != IDENT) {
                return false;
            }
            ++i;
            continue;
        }
        break;
    }
    return i < static_cast<int>(tokens.size()) &&
           tokens[static_cast<std::size_t>(i)].type == BECOMES;

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
        n->children.push_back(new ParseNode{peek().toString(), {}});  
        expect(EQL);
        n->children.push_back(parseConstDef());
        n->children.push_back(new ParseNode{peek().toString(), {}});  
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
        n->children.push_back(new ParseNode{peek().toString(), {}});  
        expect(EQL);
        n->children.push_back(parseType());
        n->children.push_back(new ParseNode{peek().toString(), {}});  
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
        n->children.push_back(new ParseNode{peek().toString(), {}});  
        expect(COLON);
        n->children.push_back(parseType());
        n->children.push_back(new ParseNode{peek().toString(), {}});  
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
    n->children.push_back(new ParseNode{"arraysy", {}});
    expect(ARRAYSY);
    n->children.push_back(new ParseNode{"lbrack", {}});
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
    n->children.push_back(new ParseNode{"rbrack", {}});
    expect(RBRACK);
    n->children.push_back(new ParseNode{"ofsy", {}});
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
    n->children.push_back(new ParseNode{"lparent", {}});
    expect(LPARENT);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(IDENT);
    }
    n->children.push_back(new ParseNode{"rparent", {}});
    expect(RPARENT);
    return n;
}

ParseNode* Parser::parseRecord() {
    auto* n = new ParseNode{"<record-type>", {}};
    n->children.push_back(new ParseNode{"recordsy", {}});
    expect(RECORDSY);
    n->children.push_back(parseFieldList());
    n->children.push_back(new ParseNode{"endsy", {}});
    expect(ENDSY);
    return n;
}

ParseNode* Parser::parseFieldList() {
    auto* n = new ParseNode{"<field-list>", {}};
    while (peek().type != ENDSY) {
        n->children.push_back(parseField());
        if (peek().type == ENDSY) break;
        n->children.push_back(new ParseNode{peek().toString(), {}});  
        expect(SEMICOLON);
    }
    return n;
}

ParseNode* Parser::parseField() {
    auto* n = new ParseNode{"<field-part>", {}};
    n->children.push_back(parseIdList());
    n->children.push_back(new ParseNode{peek().toString(), {}});  
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
    n->children.push_back(new ParseNode{"proceduresy", {}});
    expect(PROCEDURESY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    if (peek().type == LPARENT) {
        n->children.push_back(parseFormals());
    }
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(SEMICOLON);
    n->children.push_back(parseBlock());
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseFunc() {
    auto* n = new ParseNode{"<function-declaration>", {}};
    n->children.push_back(new ParseNode{"functionsy", {}});
    expect(FUNCTIONSY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    if (peek().type == LPARENT) {
        n->children.push_back(parseFormals());
    }
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(COLON);
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(IDENT);
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(SEMICOLON);
    n->children.push_back(parseBlock());
    n->children.push_back(new ParseNode{peek().toString(), {}});  
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
    n->children.push_back(new ParseNode{"lparent", {}});
    expect(LPARENT);
    n->children.push_back(parseParamGroup());
    while (peek().type == SEMICOLON) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseParamGroup());
    }
    n->children.push_back(new ParseNode{"rparent", {}});
    expect(RPARENT);
    return n;
}

ParseNode* Parser::parseParamGroup() {
    auto* n = new ParseNode{"<parameter-group>", {}};
    if (peek().type == VARSY) {
        n->children.push_back(new ParseNode{"varsy", {}});
        expect(VARSY);
    }
    n->children.push_back(parseIdList());
    n->children.push_back(new ParseNode{peek().toString(), {}});  
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
    auto* n = new ParseNode{"<compound-statement>", {}};
    n->children.push_back(new ParseNode{"beginsy", {}});
    expect(BEGINSY);
    n->children.push_back(parseStmtList());
    n->children.push_back(new ParseNode{"endsy", {}});
    expect(ENDSY);
    return n;
}

ParseNode* Parser::parseStmtList() {
    auto* n = new ParseNode{"<statement-list>", {}};
    while (peek().type != ENDSY) {
        n->children.push_back(parseStmt());
        if (peek().type == ENDSY) break;
        if (peek().type == SEMICOLON) {
            n->children.push_back(new ParseNode{consume().toString(), {}});
            continue;
        }

        Token t = peek();
        checkNotUnknown(t);
        if (t.type == IFSY || t.type == CASESY || t.type == WHILESY ||
            t.type == REPEATSY || t.type == FORSY || t.type == IDENT) {
            continue;
        }
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected semicolon or endsy");
    }
    return n;
}

ParseNode* Parser::parseStmt() {
    auto* n = new ParseNode{"<statement>", {}};
    Token t = peek();
    checkNotUnknown(t);

    if (t.type == SEMICOLON) {
        return n;
    }

    if (t.type == IFSY) {
        n->children.push_back(parseIf());
        return n;
    }
    if (t.type == CASESY) {
        n->children.push_back(parseCase());
        return n;
    }
    if (t.type == WHILESY) {
        n->children.push_back(parseWhile());
        return n;
    }
    if (t.type == REPEATSY) {
        n->children.push_back(parseRepeat());
        return n;
    }
    if (t.type == FORSY) {
        n->children.push_back(parseFor());
        return n;
    }

    if (t.type == IDENT) {
        if (isAssignFollowed(tokens, pos)) {
            n->children.push_back(parseAssign());
            return n;
        }
        n->children.push_back(parseCall());
        return n;
    }

    return n;
}

ParseNode* Parser::parseVar() {
    Token id = peek();
    checkNotUnknown(id);
    if (id.type != IDENT) {
        throw SyntaxError("Syntax error at line " + std::to_string(id.line) +
                          ": unexpected token " + id.toString() + ", expected ident in <variable>");
    }

    auto* n = new ParseNode{"<variable>", {}};
    n->children.push_back(new ParseNode{consume().toString(), {}});  

    while (peek().type == LBRACK || peek().type == PERIOD) {
        auto* cv = new ParseNode{"<component-variable>", {}};
        if (peek().type == LBRACK) {
            cv->children.push_back(new ParseNode{consume().toString(), {}});  
            cv->children.push_back(parseIndices());
            cv->children.push_back(new ParseNode{peek().toString(), {}});     
            expect(RBRACK);
        } else {
            cv->children.push_back(new ParseNode{consume().toString(), {}});  
            cv->children.push_back(new ParseNode{peek().toString(), {}});     
            expect(IDENT);
        }
        n->children.push_back(cv);
    }

    return n;
}

ParseNode* Parser::parseIndices() {
    auto* n = new ParseNode{"<index-list>", {}};
    Token x = peek();
    checkNotUnknown(x);
    if (x.type != INTCON && x.type != CHARCON && x.type != IDENT) {
        throw SyntaxError("Syntax error at line " + std::to_string(x.line) +
                          ": unexpected token " + x.toString() +
                          ", expected intcon, charcon, or ident in <index-list>");
    }
    n->children.push_back(new ParseNode{consume().toString(), {}});
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        Token y = peek();
        checkNotUnknown(y);
        if (y.type != INTCON && y.type != CHARCON && y.type != IDENT) {
            throw SyntaxError("Syntax error at line " + std::to_string(y.line) +
                              ": unexpected token " + y.toString() +
                              ", expected intcon, charcon, or ident in <index-list>");
        }
        n->children.push_back(new ParseNode{consume().toString(), {}});
    }
    return n;
}

ParseNode* Parser::parseAssign() {
    auto* n = new ParseNode{"<assignment-statement>", {}};
    n->children.push_back(parseVar());
    n->children.push_back(new ParseNode{"becomes", {}});
    expect(BECOMES);
    n->children.push_back(parseExpr());
    return n;
}

ParseNode* Parser::parseIf() {
    auto* n = new ParseNode{"<if-statement>", {}};
    n->children.push_back(new ParseNode{"ifsy", {}});
    expect(IFSY);
    n->children.push_back(parseExpr());
    n->children.push_back(new ParseNode{"thensy", {}});
    expect(THENSY);
    n->children.push_back(parseStmt());
    if (peek().type == ELSESY) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseStmt());
    }
    return n;
}

ParseNode* Parser::parseCase() {
    auto* n = new ParseNode{"<case-statement>", {}};
    n->children.push_back(new ParseNode{"casesy", {}});
    expect(CASESY);
    n->children.push_back(parseExpr());
    n->children.push_back(new ParseNode{"ofsy", {}});
    expect(OFSY);
    for (;;) {
        n->children.push_back(parseCaseArm());
        if (peek().type == SEMICOLON) {
            Token nxt = peekAt(1);
            if (canStartConstant(nxt.type)) {
                n->children.push_back(new ParseNode{consume().toString(), {}});
                continue;
            }
            if (nxt.type == ENDSY) {
                n->children.push_back(new ParseNode{consume().toString(), {}});
            }
        }
        break;
    }
    n->children.push_back(new ParseNode{"endsy", {}});
    expect(ENDSY);
    return n;
}

ParseNode* Parser::parseCaseArm() {
    auto* n = new ParseNode{"<case-block>", {}};
    n->children.push_back(parseConstDef());
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseConstDef());
    }
    n->children.push_back(new ParseNode{peek().toString(), {}});  
    expect(COLON);
    n->children.push_back(parseStmt());
    return n;
}

ParseNode* Parser::parseWhile() {
    auto* n = new ParseNode{"<while-statement>", {}};
    n->children.push_back(new ParseNode{"whilesy", {}});
    expect(WHILESY);
    n->children.push_back(parseExpr());
    n->children.push_back(new ParseNode{"dosy", {}});
    expect(DOSY);
    n->children.push_back(parseCompound());
    n->children.push_back(new ParseNode{"semicolon", {}});
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseRepeat() {
    auto* n = new ParseNode{"<repeat-statement>", {}};
    n->children.push_back(new ParseNode{"repeatsy", {}});
    expect(REPEATSY);
    auto* stmts = new ParseNode{"<statement-list>", {}};
    while (peek().type != UNTILSY) {
        stmts->children.push_back(parseStmt());
        if (peek().type == UNTILSY) break;
        if (peek().type == SEMICOLON) {
            stmts->children.push_back(new ParseNode{consume().toString(), {}});
            continue;
        }
        Token t = peek();
        checkNotUnknown(t);
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected semicolon or untilsy");
    }
    n->children.push_back(stmts);
    n->children.push_back(new ParseNode{"untilsy", {}});
    expect(UNTILSY);
    n->children.push_back(parseExpr());
    return n;
}

ParseNode* Parser::parseFor() {
    auto* n = new ParseNode{"<for-statement>", {}};
    n->children.push_back(new ParseNode{"forsy", {}});
    expect(FORSY);
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    n->children.push_back(new ParseNode{"becomes", {}});
    expect(BECOMES);
    n->children.push_back(parseExpr());
    Token t = peek();
    checkNotUnknown(t);
    if (t.type == TOSY || t.type == DOWNTOSY) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
    } else {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected tosy or downtosy");
    }
    n->children.push_back(parseExpr());
    n->children.push_back(new ParseNode{"dosy", {}});
    expect(DOSY);
    n->children.push_back(parseCompound());
    n->children.push_back(new ParseNode{"semicolon", {}});
    expect(SEMICOLON);
    return n;
}

ParseNode* Parser::parseCall() {
    auto* n = new ParseNode{"<procedure/function-call>", {}};
    n->children.push_back(new ParseNode{peek().toString(), {}});
    expect(IDENT);
    if (peek().type == LPARENT) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseArgs());
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(RPARENT);
    }
    return n;
}

ParseNode* Parser::parseArgs() {
    auto* n = new ParseNode{"<parameter-list>", {}};
    if (peek().type == RPARENT) {
        return n;
    }
    n->children.push_back(parseExpr());
    while (peek().type == COMMA) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseExpr());
    }
    return n;
}

ParseNode* Parser::parseExpr() {
    auto* n = new ParseNode{"<expression>", {}};
    ParseNode* s1 = parseSimple();
    if (isRelational(peek().type)) {
        n->children.push_back(s1);
        n->children.push_back(parseRelOp());
        n->children.push_back(parseSimple());
        return n;
    }
    n->children.push_back(s1);
    return n;
}

ParseNode* Parser::parseSimple() {
    auto* n = new ParseNode{"<simple-expression>", {}};
    if (peek().type == PLUS || peek().type == MINUS) {
        n->children.push_back(new ParseNode{peek().toString(), {}});
        consume();
    }
    n->children.push_back(parseTerm());
    while (isAdditiveOp(peek().type)) {
        n->children.push_back(parseAddOp());
        n->children.push_back(parseTerm());
    }
    return n;
}

ParseNode* Parser::parseTerm() {
    auto* n = new ParseNode{"<term>", {}};
    n->children.push_back(parseFactor());
    while (isMultiplicativeOp(peek().type)) {
        n->children.push_back(parseMulOp());
        n->children.push_back(parseFactor());
    }
    return n;
}

ParseNode* Parser::parseFactor() {
    auto* n = new ParseNode{"<factor>", {}};
    Token t = peek();
    checkNotUnknown(t);

    if (t.type == INTCON || t.type == REALCON || t.type == CHARCON || t.type == STRING) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        return n;
    }

    if (t.type == NOTSY) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseFactor());
        return n;
    }

    if (t.type == LPARENT) {
        n->children.push_back(new ParseNode{consume().toString(), {}});
        n->children.push_back(parseExpr());
        n->children.push_back(new ParseNode{peek().toString(), {}});
        expect(RPARENT);
        return n;
    }

    if (t.type == IDENT) {
        if (peekAt(1).type == LPARENT) {
            auto* call = new ParseNode{"<procedure/function-call>", {}};
            call->children.push_back(new ParseNode{t.toString(), {}});
            consume();
            call->children.push_back(new ParseNode{consume().toString(), {}});
            call->children.push_back(parseArgs());
            call->children.push_back(new ParseNode{peek().toString(), {}});
            expect(RPARENT);
            n->children.push_back(call);
            return n;
        }
        n->children.push_back(parseVar());
        return n;
    }

    throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                      ": unexpected token " + t.toString() + " in <factor>");
}

ParseNode* Parser::parseRelOp() {
    auto* n = new ParseNode{"<relational-operator>", {}};
    Token t = peek();
    checkNotUnknown(t);
    if (!isRelational(t.type)) {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected relational operator");
    }
    n->children.push_back(new ParseNode{consume().toString(), {}});
    return n;
}

ParseNode* Parser::parseAddOp() {
    auto* n = new ParseNode{"<additive-operator>", {}};
    Token t = peek();
    checkNotUnknown(t);
    if (!isAdditiveOp(t.type)) {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() + ", expected additive operator");
    }
    n->children.push_back(new ParseNode{consume().toString(), {}});
    return n;
}

ParseNode* Parser::parseMulOp() {
    auto* n = new ParseNode{"<multiplicative-operator>", {}};
    Token t = peek();
    checkNotUnknown(t);
    if (!isMultiplicativeOp(t.type)) {
        throw SyntaxError("Syntax error at line " + std::to_string(t.line) +
                          ": unexpected token " + t.toString() +
                          ", expected multiplicative operator");
    }
    n->children.push_back(new ParseNode{consume().toString(), {}});
    return n;
}

namespace {

void printTreeImpl(std::ostream& os, ParseNode* node, const std::string& prefix, bool isLast, bool isRoot) {
    if (!node) return;
    if (isRoot) {
        os << node->label << '\n';
    } else {
        os << prefix << (isLast ? "\\-- " : "+-- ") << node->label << '\n';
    }
    const std::string nextPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    for (std::size_t i = 0; i < node->children.size(); ++i) {
        printTreeImpl(os, node->children[i], nextPrefix, i + 1 == node->children.size(), false);
    }
}

}

void printTree(ParseNode* root) {
    if (!root) return;
    printTreeImpl(std::cout, root, "", true, true);
}

void writeTree(ParseNode* root, const std::string& outputPath) {
    if (!root) return;

    namespace fs = std::filesystem;
    fs::path outDir = fs::path(outputPath).parent_path();
    if (!outDir.empty() && !fs::exists(outDir))
        fs::create_directories(outDir);

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        throw std::runtime_error("cannot open output file: " + outputPath);
    }
    printTreeImpl(out, root, "", true, true);
}

void destroyParseTree(ParseNode* root) {
    if (!root) return;
    for (ParseNode* child : root->children)
        destroyParseTree(child);
    delete root;
}