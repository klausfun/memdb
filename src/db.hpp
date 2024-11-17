#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "table.hpp"
#include "result.hpp"
#include "parser.hpp"
#include "command_registry.hpp"

class Database {
public:
    Database();
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;

    Result execute(const std::string& query);
    void load_from_file(std::ifstream& file);
    void save_to_file(std::ofstream& file) const;

private:
    friend class Command;
    std::unordered_map<std::string, std::shared_ptr<Table>> tables;
    CommandRegistry command_registry;
};
