#include "../header/lexer.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_map>
using namespace std;


/*buat baca file*/
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
            ts.push_back(readWord()); // ini nanti bisa cek identifier atau keyword
        } else if(checkDigit(currChar)){
            ts.push_back(readNum()); // pas read num bakalan ada bagian untuk baca real
        } else if(currChar == '\''){
            ts.push_back(readStr()); // ini baca char atau string jadinya
        } else if(currChar == '{') {
            Token t = readComment();
            if (t.type != ArionToken::COMMENT) {
                ts.push_back(t);
            }
        } else if(currChar == '('){
            char n = f.peek();
            if(n == '*'){
                Token t = readComment();
                if (t.type != ArionToken::COMMENT) {
                    ts.push_back(t);
                }
            } else {
                ts.push_back(Token{ArionToken::LPARENT, string(1, currChar), currLine});
                next();
            }
        } else if (currChar == '+'){
            ts.push_back(Token(ArionToken::PLUS, string(1, currChar), currLine));
            next();
        } else if (currChar == '-'){
            ts.push_back(Token(ArionToken::MINUS, string(1, currChar), currLine));
            next();
        } else if (currChar == '*'){
            ts.push_back(Token(ArionToken::TIMES, string(1, currChar), currLine));
            next();
        } else if (currChar == '/'){
            ts.push_back(Token(ArionToken::RDIV, string(1, currChar), currLine));
            next();
        } else if (currChar == '='){
            char n = f.peek();
            if (n == '=') {
                next(); // konsumsi '=' pertama
                next(); // konsumsi '=' kedua
                ts.push_back(Token(ArionToken::EQL, "==", currLine));
            } else {
                // '=' sendirian = tidak valid
                ts.push_back(Token(ArionToken::UNKNOWN, "=", currLine));
                next();
            }
        } else if(currChar == ':'){
            char n = f.peek();
            if (n == '='){
                next();
                next();
                ts.push_back(Token(ArionToken::BECOMES, ":=", currLine));
            } else {
                ts.push_back(Token(ArionToken::COLON, ":", currLine));
                next();
            }
        } else if (currChar == '<'){
            char n = f.peek();
            if (n == '>') {
                next();
                next();
                ts.push_back(Token(ArionToken::NEQ, "<>", currLine));
            } else if (n == '='){
                next();
                next();
                ts.push_back(Token(ArionToken::LEQ, "<=", currLine));
            } else {
                ts.push_back(Token(ArionToken::LSS, "<", currLine));
                next();
            }
        } else if (currChar == '>'){
            char n = f.peek();
            if (n == '='){
                next();
                next();
                ts.push_back(Token(ArionToken::GEQ, ">=", currLine));
            } else {
                ts.push_back(Token(ArionToken::GTR, ">", currLine));
                next();
            }
        } else if (currChar == ')'){
            ts.push_back(Token(ArionToken::RPARENT, ")", currLine));
            next();
        } else if (currChar == '['){
            ts.push_back(Token(ArionToken::LBRACK, "[", currLine));
            next();
        } else if (currChar == ']'){
            ts.push_back(Token(ArionToken::RBRACK, "]", currLine));
            next();
        } else if (currChar == ','){
            ts.push_back(Token(ArionToken::COMMA, ",", currLine));
            next();
        } else if (currChar == ';'){
            ts.push_back(Token(ArionToken::SEMICOLON, ";", currLine));
            next();
        } else if (currChar == '.'){
            // ".digit" → bukan angka valid (Arion tidak mengizinkan real tanpa
            // angka pra-titik); konsumsi sebagai satu token UNKNOWN (longest-match).
            if (checkDigit(f.peek())) {
                string buffer(1, currChar);
                int startLine = currLine;
                next();
                ts.push_back(readMalformedNumber(buffer, startLine));
            } else {
                ts.push_back(Token(ArionToken::PERIOD, ".", currLine));
                next();
            }
        }
        else {
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

Token Lexer::readWord(){
    string buffer = "";
    int startLine = currLine;

    while (checkAlpha(currChar) || checkDigit(currChar)) {
        buffer += currChar;
        next();
    }

    string lower = buffer;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    ArionToken type = checkWord(lower);
    if (type == IDENT){
        return Token(IDENT, buffer, startLine);
    } else {
        return Token(type, "", startLine);
    }
}

Token Lexer::readNum(){
    string buffer = "";
    int startLine = currLine;

    while (checkDigit(currChar)) {
        buffer += currChar;
        next();
    }

    if (currChar == '.'){
        // ".." operator range (mis. 1..10), bukan bagian dari angka
        if (f.peek() == '.') {
            return Token(ArionToken::INTCON, buffer, startLine);
        }
        buffer += currChar;
        next();
        if (!checkDigit(currChar)){
            return readMalformedNumber(buffer, startLine);
        }
        while (checkDigit(currChar)){
            buffer += currChar;
            next();
        }
        if (currChar == '.' || checkAlpha(currChar)) {
            return readMalformedNumber(buffer, startLine);
        }
        return Token(ArionToken::REALCON, buffer, startLine);
    } else {
        return Token(ArionToken::INTCON, buffer, startLine);
    }
}

Token Lexer::readMalformedNumber(string buffer, int startLine) {
    while (currChar != EOF) {
        bool numLike = checkAlpha(currChar) || checkDigit(currChar)
                       || currChar == '.' || currChar == '+'
                       || currChar == '-' || currChar == '/';
        if (!numLike) break;
        buffer += currChar;
        next();
    }
    return Token(ArionToken::UNKNOWN, buffer, startLine);
}

Token Lexer::readStr(){
    string buffer = "";
    int startLine = currLine;
    
    buffer += currChar; // petik pembuka '
    next();
    
    int count = 0;
    while (currChar != '\'' && currChar != EOF) {
        buffer += currChar;
        next();
        count++;
    }

    if (currChar == EOF) {
        // string ga ditutup sebelum EOF
        return Token(ArionToken::UNKNOWN, buffer, startLine);
    }

    // konsumsi petik penutup
    buffer += currChar; // masukin ' penutup ke buffer
    next();

    if (count == 0) {
        return Token(ArionToken::UNKNOWN, buffer, startLine); // '' kosong = error
    } else if (count == 1) {
        return Token(ArionToken::CHARCON, buffer, startLine);
    } else {
        return Token(ArionToken::STRING, buffer, startLine);
    }
}

Token Lexer::readComment(){
    int startLine = currLine;
    
    if (currChar == '{') {
        next(); 
        while (currChar != '}' && currChar != EOF) {
            next();
        }
        if (currChar == '}') {
            next(); 
        } else {
            return Token(ArionToken::UNKNOWN, "unclosed comment", startLine);
        }
        return Token(ArionToken::COMMENT, "", startLine);
        
    } else {
        next(); 
        next(); 
        while (currChar != EOF) {
            if (currChar == '*') {
                next();
                if (currChar == ')') {
                    next(); 
                    break;
                }
            } else {
                next();
            }
        }
        if (currChar == EOF) {
            return Token(ArionToken::UNKNOWN, "unclosed comment", startLine);
        }
        return Token(ArionToken::COMMENT, "", startLine);
    }
}

bool Lexer::checkAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::checkDigit(char c){
    return (c >= '0' && c <= '9');
}

bool Lexer::checkSpace(char c){
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}