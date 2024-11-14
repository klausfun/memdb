#include "db.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>

Database::Database() {
    command_registry.register_command("create", []() {
        return std::make_unique<CreateCommand>();
    });
    command_registry.register_command("insert", []() {
        return std::make_unique<InsertCommand>();
    });
    command_registry.register_command("select", []() {
        return std::make_unique<SelectCommand>();
    });
}

//void Database::load_from_file(std::ifstream& file) {
//    std::string line;
//    while (std::getline(file, line)) {
//        std::istringstream ss(line);
//        std::string table_name;
//        ss >> table_name;
//
//        tables[table_name] = std::make_shared<Table>(table_name);
//    }
//}
//
//void Database::save_to_file(std::ofstream& file) const {
//    for (const auto& pair : tables) {
//        const auto& table = pair.second;
//        file << table->get_name() << std::endl;
//    }
//}

//std::string to_lower(const std::string& str) {
//    std::string lower_str = str;
//    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
//    return lower_str;
//}
//
//std::string sanitize_query(const std::string& query) {
//    std::string sanitized = query;
//    std::regex ws_re("\\s+");
//    sanitized = std::regex_replace(sanitized, ws_re, " ");
//
//    return sanitized;
//}

Result Database::execute(const std::string& query) {
    auto command = command_registry.get_command(query.substr(0, query.find(" ")));

    if (command) {
        return command->execute(*this, query);
    } else {
        std::cerr << "Error: Unsupported query type" << std::endl;
        return Result{};
    }
}