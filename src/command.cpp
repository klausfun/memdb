#include <iostream>
#include "command.h"

Result CreateCommand::execute(Database& db, const std::string& query) {
    // Добавьте логику выполнения команды CREATE
    std::cout << "Executing CREATE command: " << query << std::endl;
    return Result{};
}

Result InsertCommand::execute(Database& db, const std::string& query) {
    // Добавьте логику выполнения команды INSERT
    std::cout << "Executing INSERT command: " << query << std::endl;
    return Result{};
}

Result SelectCommand::execute(Database& db, const std::string& query) {
    // Добавьте логику выполнения команды SELECT
    std::cout << "Executing SELECT command: " << query << std::endl;
    return Result{};
}