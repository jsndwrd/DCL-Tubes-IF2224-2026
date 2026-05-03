#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "header/lexer.hpp"
#include "header/parser.hpp"
#include "header/token.hpp"

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
    cout << "  ARION LEXER & PARSER  -  Milestone 2\n";
    cout << "  IF2224 Teori Bahasa Formal dan Automata\n";
    cout << "  Kelompok DCL - Dirty Chocolate\n";
    cout << "===================================================\n\n";

    string inputPath;

    while (true) {
        cout << "Masukkan path berkas dari folder test/ (contoh: milestone-2/input-1.txt): ";
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

    ParseNode* root = nullptr;
    try {
        Parser parser(tokens);
        root = parser.parse();
    } catch (const SyntaxError& e) {
        cerr << "\n" << e.what() << "\n";
        return 1;
    }

    cout << "\n--- Pohon Parsing ---\n";
    printTree(root);
    cout << "----------------------\n";

    cout << "\nSimpan pohon parsing ke berkas .txt? (y/n): ";
    string jawab;
    if (!getline(cin, jawab)) {
        cout << "\nProgram dihentikan.\n";
        destroyParseTree(root);
        return 0;
    }

    if (jawab != "y" && jawab != "Y") {
        cout << "Pohon tidak disimpan.\n\n";
        destroyParseTree(root);
        return 0;
    }

    string outputPath;
    while (true) {
        cout << "Masukkan path keluaran dari folder test/ (contoh: milestone-2/output-tree-1.txt): ";
        string raw;
        if (!getline(cin, raw)) {
            cout << "\nProgram dihentikan.\n";
            destroyParseTree(root);
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
        writeTree(root, outputPath);
        cout << "Pohon parsing disimpan ke: " << outputPath << "\n\n";
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        destroyParseTree(root);
        return 1;
    }

    destroyParseTree(root);
    return 0;
}
