#include <iostream>
#include "command.h"

Result CreateTableCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Create Table command:\n" << std::endl;
    return Result{};
}

Result InsertCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing INSERT command:\n" << std::endl;
    return Result{};
}

Result SelectCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing SELECT command:\n" << std::endl;
    return Result{};
}

Result UpdateCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Update command:\n" << std::endl;
    return Result{};
}

Result DeleteCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Delete command:\n" << std::endl;
    return Result{};
}

Result CreateIndexCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Create Index command:\n" << std::endl;
    return Result{};
}