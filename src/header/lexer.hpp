#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "token.hpp"

class Lexer {
public:
    Lexer(const std::string& filename);

    // Convert file strings into token and its values
    std::vector<Token> tokenize();

private:
    std::ifstream f;
    std::string fname;
    int currLine;
    char currChar;

    // Continue to next char
    void next();

    // Skip whitespace
    void skip();
    
    // Check case-insensitive reserved keyword
    // Return reserved token or IDENT
    ArionToken checkWord(const std::string& w);

    // Read case-insensitive word char by char
    // Return identifier token or keyword (reserved) token
    Token readWord();

    // Read numbers
    // Return INTCON or REALCON
    Token readNum();

    // Read char with ''
    // Return CHARCON or STRING
    Token readStr();

    // Read char in { } or (* *)
    // Return COMMENT or an Error Token (unclosed comment)
    Token readComment();

    // Checker
    bool checkAlpha(char c);
    bool checkDigit(char c);
    bool checkSpace(char c);

};

#endif