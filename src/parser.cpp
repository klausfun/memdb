#include <iostream>
#include <cctype>
#include "parser.hpp"

void Tokenizer::printTokens(const std::vector<std::string>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Value: " << token << std::endl;
    }
}

std::vector<std::string> Tokenizer::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string buffer;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (std::isspace(c)) {
            if (!buffer.empty()) {
                tokens.push_back(buffer);
                buffer.clear();
            }
            continue;
        }

        if (c == '{' || c == '}' || c == '[' || c == ']' || c == ',' || c == ':' || c == '(' || c == ')' || c == '=') {
            if (!buffer.empty()) {
                tokens.push_back(buffer);
                buffer.clear();
            }
            tokens.push_back(std::string(1, c));
            continue;
        }

        if (std::isalpha(c) || c == '_' || c == '"' || c == '$' || std::isdigit(c)) {
            buffer += c;

            if (i + 1 == input.size() || !(std::isalnum(input[i + 1]) ||
                input[i + 1] == '_' || input[i + 1] == '"' || input[i + 1] == '$')) {

                if (buffer == "create" || buffer == "table" || buffer == "select" || buffer == "insert" ||
                    buffer == "update" || buffer == "delete" || buffer == "join" || buffer == "on" ||
                    buffer == "where" || buffer == "set" || buffer == "by") {
                    tokens.push_back(buffer);
                } else {
                    tokens.push_back(buffer);
                }

                buffer.clear();
            }
        } else {
            if (!buffer.empty()) {
                tokens.push_back(buffer);
                buffer.clear();
            }
            tokens.push_back(std::string(1, c));
        }
    }

    if (!buffer.empty()) {
        tokens.push_back(buffer);
    }

    return tokens;
}