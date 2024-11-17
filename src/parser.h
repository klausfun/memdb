#include <vector>
#include <string>

struct Token {
    std::string type;
    std::string value;
};

class Tokenizer {
public:
    Tokenizer() = default;

    std::vector<Token> tokenize(const std::string& input);
    void printTokens(const std::vector<Token>& tokens);

};