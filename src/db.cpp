#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include "db.h"

Database::Database() {
    command_registry.register_command("create table", []() {
        return std::make_unique<CreateTableCommand>();
    });
    command_registry.register_command("insert", []() {
        return std::make_unique<InsertCommand>();
    });
    command_registry.register_command("select", []() {
        return std::make_unique<SelectCommand>();
    });
    command_registry.register_command("update", []() {
        return std::make_unique<UpdateCommand>();
    });
    command_registry.register_command("delete", []() {
        return std::make_unique<DeleteCommand>();
    });
    command_registry.register_command("create index", []() {
        return std::make_unique<CreateIndexCommand>();
    });
}

//void Database::save_to_file(std::ofstream& file) const {
//    // Write format version
//    uint32_t version = 1;
//    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
//
//    // Write number of tables
//    uint32_t table_count = tables.size();
//    file.write(reinterpret_cast<const char*>(&table_count), sizeof(table_count));
//
//    // Write each table
//    for (const auto& [name, table] : tables) {
//        // Write table name
//        uint32_t name_length = name.length();
//        file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
//        file.write(name.c_str(), name_length);
//
//        // Write table data
//        table->serialize(file);
//    }
//}
//
//void Database::load_from_file(std::ifstream& file) {
//    // Clear existing tables
//    tables.clear();
//
//    // Read and verify format version
//    uint32_t version;
//    file.read(reinterpret_cast<char*>(&version), sizeof(version));
//    if (version != 1) {
//        throw std::runtime_error("Unsupported file format version");
//    }
//
//    // Read number of tables
//    uint32_t table_count;
//    file.read(reinterpret_cast<char*>(&table_count), sizeof(table_count));
//
//    // Read each table
//    for (uint32_t i = 0; i < table_count; ++i) {
//        // Read table name
//        uint32_t name_length;
//        file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
//        std::string name(name_length, '\0');
//        file.read(&name[0], name_length);
//
//        // Create and load table
//        auto table = std::make_shared<Table>(name);
//        table->deserialize(file);
//        tables[name] = table;
//    }
//}

std::string to_lower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

Result Database::execute(const std::string& query) {
    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(query);
    if (tokens.empty()) {
//        return Result::error("Empty query");
        std::cerr << "Empty query" << std::endl;
        return Result{};
    }

    std::string keyword = to_lower(tokens[0].value);
    if (keyword == "create") {
        if (tokens.size() > 1) {
            if (to_lower(tokens[1].value) == "table") {
                keyword += " table";
            } else if (to_lower(tokens[2].value) == "index") {
                keyword += " index";
            } else {
                std::cerr << "Error: Unsupported query type" << std::endl;
                return Result{};
            }
        } else {
//            return Result::error("Empty query");
            std::cerr << "Empty query" << std::endl;
            return Result{};
        }
    }

    auto command = command_registry.get_command(keyword);
    if (!command) {
//        return Result::error("Unknown command: " + keyword);
        return Result{};
    }

    return command->execute(*this, tokens);
}