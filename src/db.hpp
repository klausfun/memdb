#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "table.hpp"
#include "result.hpp"
#include "parser.hpp"
#include "command/command_registry.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Database {
public:
    Database();
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;

    Result execute(const std::string& query);
    void save_to_json(const std::string& filename);
    void load_from_json(const std::string& filename);
//    void load_from_file(std::ifstream& file);
//    void save_to_file(std::ofstream& file) const;

private:
    friend class CreateTableCommand;
    friend class InsertCommand;
    friend class Command;

    void createTable(const std::string& name, const std::vector<Column>& columns);
    std::shared_ptr<Table> getTable(const std::string& name);

    std::unordered_map<std::string, std::shared_ptr<Table>> tables;
    CommandRegistry command_registry;
};
