#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "header/lexer.hpp"
#include "header/parser.hpp"
#include "header/token.hpp"
#include "header/ast.hpp"
#include "header/symtab.hpp"

using namespace std;
namespace fs = std::filesystem;

string formatToken(const Token& t) {
    switch (t.type) {
        case IDENT:
        case INTCON:
        case REALCON:
        case CHARCON:
        case STRING:
        case UNKNOWN:
            if (!t.value.empty()) return t.name() + "(" + t.value + ")";
            return t.name();
        default:
            return t.name();
    }
}

void printSummary(const vector<Token>& tokens) {
    int total = 0, errors = 0, idents = 0, keywords = 0, literals = 0;

    for (const Token& t : tokens) {
        if (t.type == EOFILE) continue;
        total++;
        if (t.type == UNKNOWN) errors++;
        if (t.type == IDENT)   idents++;
        if (t.type == INTCON || t.type == REALCON ||
            t.type == CHARCON || t.type == STRING) literals++;
        if (t.type != IDENT   && t.type != INTCON  && t.type != REALCON &&
            t.type != CHARCON && t.type != STRING  && t.type != UNKNOWN)
            keywords++;
    }

    cout << "\n";
    cout << "Total token      : " << total    << "\n";
    cout << "Kata kunci       : " << keywords << "\n";
    cout << "Identifier       : " << idents   << "\n";
    cout << "Literal          : " << literals << "\n";
    cout << "Token tidak valid: " << errors   << "\n";

    if (errors > 0) {
        cout << "\nToken tidak dikenali:\n";
        for (const Token& t : tokens) {
            if (t.type == UNKNOWN)
                cout << "  Baris " << t.line << ": '" << t.value << "'\n";
        }
    }
}

void printTokens(const vector<Token>& tokens) {
    cout << "\n--- Daftar Token ---\n";
    int prevLine = -1;
    for (const Token& t : tokens) {
        if (t.type == EOFILE) break;
        if (prevLine != -1 && t.line > prevLine + 1)
            cout << "\n";
        cout << formatToken(t) << "\n";
        prevLine = t.line;
    }
    cout << "--------------------\n";
}

int main() {
    cout << "===================================================\n";
    cout << "  IF2224 Teori Bahasa Formal dan Automata\n";
    cout << "  Kelompok DCL - Dirty Chocolate\n";
    cout << "===================================================\n\n";

    string inputPath;

    while (true) {
        cout << "Masukkan path berkas dari folder test/: ";
        string raw;
        if (!getline(cin, raw)) {
            cout << "\nProgram dihentikan.\n";
            return 0;
        }

        if (raw.empty()) {
            cout << "ERROR: Path berkas tidak boleh kosong.\n";
            continue;
        }

        inputPath = "test/" + raw;
        if (!fs::exists(inputPath)) {
            cout << "ERROR: Berkas tidak ditemukan: " << inputPath << "\n";
        } else {
            break;
        }
    }

    cout << "Memproses " << inputPath << " ...\n";

    Lexer lexer(inputPath);
    vector<Token> tokens = lexer.tokenize();

    printTokens(tokens);
    printSummary(tokens);

    ParseNode* parseRoot = nullptr;
    try {
        Parser parser(tokens);
        parseRoot = parser.parse();
    } catch (const SyntaxError& e) {
        cerr << "\n" << e.what() << "\n";
        return 1;
    }

    cout << "\n--- Pohon Parsing ---\n";
    printTree(parseRoot);
    cout << "----------------------\n";

    ASTNode* astRoot = buildAST(parseRoot);
    if (!astRoot) {
        cerr << "ERROR: Gagal membangun AST dari parse tree.\n";
        destroyParseTree(parseRoot);
        return 1;
    }

    try {
        SemanticAnalyzer analyzer;
        analyzer.analyze(astRoot);
    } catch (const SemanticError& e) {
        cerr << "\n" << e.what() << "\n";
        destroyAST(astRoot);
        destroyParseTree(parseRoot);
        return 1;
    }

    cout << "\n--- Decorated AST ---\n";
    printAST(astRoot);
    cout << "----------------------\n";

    {
        SymbolTable dummy; 

        SemanticAnalyzer printer;
        printer.analyze(astRoot);
        printer.sym.printTab();
        printer.sym.printBTab();
        printer.sym.printATab();
    }

    cout << "\nSimpan hasil ke berkas .txt? (y/n): ";
    string jawab;
    if (!getline(cin, jawab)) {
        cout << "\nProgram dihentikan.\n";
        destroyAST(astRoot);
        destroyParseTree(parseRoot);
        return 0;
    }

    if (jawab != "y" && jawab != "Y") {
        cout << "Hasil tidak disimpan.\n\n";
        destroyAST(astRoot);
        destroyParseTree(parseRoot);
        return 0;
    }

    string outputPath;
    while (true) {
        cout << "Masukkan path keluaran dari folder test/ (contoh: milestone-3/output-1.txt): ";
        string raw;
        if (!getline(cin, raw)) {
            cout << "\nProgram dihentikan.\n";
            destroyAST(astRoot);
            destroyParseTree(parseRoot);
            return 0;
        }
        if (raw.empty()) {
            cout << "ERROR: Path keluaran tidak boleh kosong.\n";
            continue;
        }
        outputPath = "test/" + raw;
        break;
    }

    try {
        writeAST(astRoot, outputPath);
        cout << "Decorated AST disimpan ke: " << outputPath << "\n\n";
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        destroyAST(astRoot);
        destroyParseTree(parseRoot);
        return 1;
    }

    destroyAST(astRoot);
    destroyParseTree(parseRoot);
    return 0;
}