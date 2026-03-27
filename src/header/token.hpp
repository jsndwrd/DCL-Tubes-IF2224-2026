#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum ArionToken {
    INTCON,
    REALCON,
    CHARCON,
    STRING,
    NOTSY,
    PLUS,
    MINUS,
    TIMES,
    IDIV,
    RDIV,
    IMOD,
    ANDSY,
    ORSY,
    EQL,
    NEQ,
    GTR,
    GEQ,
    LSS,
    LEQ,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    COMMA,
    SEMICOLON,
    PERIOD,
    COLON,
    BECOMES,
    CONSTSY,
    TYPESY,
    VARSY,
    FUNCTIONSY,
    PROCEDURESY,
    ARRAYSY,
    RECORDSY,
    PROGRAMSY,
    IDENT,
    BEGINSY,
    IFSY,
    CASESY,
    REPEATSY,
    WHILESY,
    FORSY,
    ENDSY,
    ELSESY,
    UNTILSY,
    OFSY,
    DOSY,
    TOSY,
    DOWNTOSY,
    THENSY,
    COMMENT,
    EOFILE,
    UNKNOWN // sentinel
};

const char* ArionTokenName[] = {
    "intcon",
    "realcon",
    "charcon",
    "string",
    "notsy",
    "plus",
    "minus",
    "times",
    "idiv",
    "rdiv",
    "imod",
    "andsy",
    "orsy",
    "eql",
    "neq",
    "gtr",
    "geq",
    "lss",
    "leq",
    "lparent",
    "rparent",
    "lbrack",
    "rbrack",
    "comma",
    "semicolon",
    "period",
    "colon",
    "becomes",
    "constsy",
    "typesy",
    "varsy",
    "functionsy",
    "proceduresy",
    "arraysy",
    "recordsy",
    "programsy",
    "ident",
    "beginsy",
    "ifsy",
    "casesy",
    "repeatsy",
    "whilesy",
    "forsy",
    "endsy",
    "elsesy",
    "untilsy",
    "ofsy",
    "dosy",
    "tosy",
    "downtosy",
    "thensy",
    "comment",
    "eofile",
    "unknown"
};

class Token {
public:
    ArionToken type;
    std::string value;
    int line;

    Token(ArionToken type, std::string value, int line) : type(type), value(value), line(line) {}

    std::string name() const {
        if(type < 0 || type > UNKNOWN) return "undefined";
        return ArionTokenName[type];
    }

    std::string toString() const {
        if (value.empty()) return name();
        return name() + " (" + value + ")";
    }

};

#endif