#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "table.h"
#include "result.h"
#include "command_registry.h"

class Database {
public:
    Database();

    void load_from_file(std::ifstream& file);
    void save_to_file(std::ofstream& file) const;

    Result execute(const std::string& query);

private:
    CommandRegistry command_registry;
    std::unordered_map<std::string, std::shared_ptr<Table>> tables;
};
