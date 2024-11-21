#pragma once
#include <vector>
#include <string>

class Tokenizer {
public:
    Tokenizer() = default;

    std::vector<std::string> tokenize(const std::string& input);
    void printTokens(const std::vector<std::string>& tokens);

};