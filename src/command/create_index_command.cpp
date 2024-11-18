#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

Result CreateIndexCommand::execute(Database& db, const std::vector<Token>& tokens, const std::string& query) {
    std::cout << "Executing Create Index command:\n" << std::endl;
    return Result{};
}