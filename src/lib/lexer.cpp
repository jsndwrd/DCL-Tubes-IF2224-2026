#include "../header/lexer.hpp"
#include <iostream>
using namespace std;

Lexer::Lexer(const std::string& fn){
    fname = fn;
    currLine = 1;
    f.open(fn);
    if(!f.is_open()){
        cerr << "[ERROR] Can't open file '" << fn << "'\n";
        exit(1);
    }
    next();
}

vector<Token> Lexer::tokenize(){
    vector<Token> ts;
    
    while(true){
        skip();
        if(currChar == EOF){
            break;
        }
        if(checkAlpha(currChar)){
            ts.push_back(readWord());
        } else if(checkDigit(currChar)){
            ts.push_back(readNum());
        } else if(currChar == '\''){
            ts.push_back(readStr());
        } else if(currChar == '{') {
            ts.push_back(readComment());
        } else if(currChar == '('){
            char n = f.peek();
            if(n == '*'){
                ts.push_back(readComment());
            } else {
                ts.push_back(Token{ArionToken::LPARENT, string(1, currChar), currLine});
                next();
            }
        } else {
            ts.push_back(Token{ArionToken::UNKNOWN, string(1, currChar), currLine});
            next();
        }
    }

    ts.push_back(Token{ArionToken::EOFILE, "", currLine});
    return ts;
}

void Lexer::next() {
    if(f.get(currChar)) {
        if(currChar == '\n'){
            currLine++;
        }
    } else {
        currChar = EOF;
    }
}

void Lexer::skip() {
    while(currChar != EOF && checkSpace(currChar)){
        next();
    }
}

ArionToken Lexer::checkWord(const string& w){
    static const unordered_map<string, ArionToken> kws = {
        {"not", NOTSY},
        {"div", IDIV},
        {"mod", IMOD},
        {"and", ANDSY},
        {"or", ORSY},
        {"const", CONSTSY},
        {"type", TYPESY},
        {"var", VARSY},
        {"function", FUNCTIONSY},
        {"procedure", PROCEDURESY},
        {"array", ARRAYSY},
        {"record", RECORDSY},
        {"program", PROGRAMSY},
        {"begin", BEGINSY},
        {"if", IFSY},
        {"case", CASESY},
        {"repeat", REPEATSY},
        {"while", WHILESY},
        {"for", FORSY},
        {"end", ENDSY},
        {"else", ELSESY},
        {"until", UNTILSY},
        {"of", OFSY},
        {"do", DOSY},
        {"to", TOSY},
        {"downto", DOWNTOSY},
        {"then", THENSY},
    };

    auto res = kws.find(w);
    if(res != kws.end()){
        return res->second;
    }
    return IDENT;
}

Token Lexer::readWord(){}

Token Lexer::readNum(){}

Token Lexer::readStr(){}

Token Lexer::readComment(){}

bool Lexer::checkAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::checkDigit(char c){
    return (c >= '0' && c <= '9');
}

bool Lexer::checkSpace(char c){
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}