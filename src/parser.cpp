#include <iostream>
#include <cctype>
#include "parser.h"

void Tokenizer::printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Type: " << token.type << ", Value: " << token.value << std::endl;
    }
}

std::vector<Token> Tokenizer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string buffer;
    bool isIdentifier = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (std::isspace(c)) {
            if (!buffer.empty()) {
                tokens.push_back({isIdentifier ? "IDENTIFIER" : "SYMBOL", buffer});
                buffer.clear();
            }
            continue;
        }

        if (c == '{' || c == '}' || c == '[' || c == ']' || c == ',' || c == ':' || c == '(' || c == ')' || c == '=') {
            if (!buffer.empty()) {
                tokens.push_back({isIdentifier ? "IDENTIFIER" : "SYMBOL", buffer});
                buffer.clear();
            }
            tokens.push_back({"SYMBOL", std::string(1, c)});
            continue;
        }

        if (std::isalpha(c) || c == '_' || c == '"' || c == '$' || std::isdigit(c)) {
            buffer += c;
            isIdentifier = true;

            if (i + 1 == input.size() || !(std::isalnum(input[i + 1]) ||
                input[i + 1] == '_' || input[i + 1] == '"' || input[i + 1] == '$')) {
                // Проверка, если это ключевое слово
                if (buffer == "create" || buffer == "table" || buffer == "select" || buffer == "insert" ||
                    buffer == "update" || buffer == "delete" || buffer == "join" || buffer == "on" ||
                    buffer == "where" || buffer == "set" || buffer == "by") {
                    tokens.push_back({"KEYWORD", buffer});
                } else {
                    tokens.push_back({"IDENTIFIER", buffer});
                }
                buffer.clear();
                isIdentifier = false;
            }
        } else {
            if (!buffer.empty()) {
                tokens.push_back({isIdentifier ? "IDENTIFIER" : "SYMBOL", buffer});
                buffer.clear();
            }
            tokens.push_back({"UNKNOWN", std::string(1, c)});
        }
    }

    if (!buffer.empty()) {
        tokens.push_back({isIdentifier ? "IDENTIFIER" : "SYMBOL", buffer});
    }

    return tokens;
}