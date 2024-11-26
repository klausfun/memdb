#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

Result CreateIndexCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    return Result{};
}