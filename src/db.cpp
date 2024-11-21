#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include "db.hpp"
#include <fstream>

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

void Database::createTable(const std::string& name, const std::vector<Column>& columns) {
    if (tables.find(name) != tables.end()) {
        throw std::runtime_error("Table '" + name + "' already exists");
    }

    tables[name] = std::make_shared<Table>(name, columns);
}

std::shared_ptr<Table> Database::getTable(const std::string& name) {
    auto it = tables.find(name);
    if (it != tables.end()) {
        return it->second;
    }

    return nullptr;
}

void Database::save_to_json(const std::string& filename) {
    json j;

    for (const auto& [table_name, table] : tables) {
        json table_json;

        json columns_json;
        for (const auto& col : table->get_columns()) {
            json column;
            column["name"] = col.name;
            column["type"] = static_cast<int>(col.type.getType());
            column["size"] = col.size;
            column["is_key"] = col.is_key;
            column["is_unique"] = col.is_unique;
            column["is_autoincrement"] = col.is_autoincrement;

            // Обработка значения по умолчанию
            if (!std::holds_alternative<std::monostate>(col.default_value)) {
                if (std::holds_alternative<int32_t>(col.default_value)) {
                    column["default_value"] = std::get<int32_t>(col.default_value);
                }
                else if (std::holds_alternative<bool>(col.default_value)) {
                    column["default_value"] = std::get<bool>(col.default_value);
                }
                else if (std::holds_alternative<std::string>(col.default_value)) {
                    column["default_value"] = std::get<std::string>(col.default_value);
                }
                else if (std::holds_alternative<std::vector<uint8_t>>(col.default_value)) {
                    const auto& bytes = std::get<std::vector<uint8_t>>(col.default_value);
                    std::stringstream ss;
                    ss << "0x";
                    for (auto byte : bytes) {
                        ss << std::hex << std::setw(2) << std::setfill('0')
                           << static_cast<int>(byte);
                    }
                    column["default_value"] = ss.str();
                }
            }

            columns_json.push_back(column);
        }
        table_json["columns"] = columns_json;

        // Сохраняем данные
        json rows_json;
        for (const auto& row : table->get_rows()) {
            json row_json;
            for (size_t i = 0; i < row.size(); ++i) {
                const auto& col = table->get_columns()[i];
                const auto& value = row[i];

                if (std::holds_alternative<int32_t>(value)) {
                    row_json[col.name] = std::get<int32_t>(value);
                }
                else if (std::holds_alternative<bool>(value)) {
                    row_json[col.name] = std::get<bool>(value);
                }
                else if (std::holds_alternative<std::string>(value)) {
                    row_json[col.name] = std::get<std::string>(value);
                }
                else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
                    const auto& bytes = std::get<std::vector<uint8_t>>(value);
                    std::stringstream ss;
                    ss << "0x";
                    for (auto byte : bytes) {
                        ss << std::hex << std::setw(2) << std::setfill('0')
                           << static_cast<int>(byte);
                    }
                    row_json[col.name] = ss.str();
                }
            }
            rows_json.push_back(row_json);
        }
        table_json["rows"] = rows_json;

        j[table_name] = table_json;
    }

    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

void Database::load_from_json(const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;

    tables.clear();

    for (const auto& [table_name, table_json] : j.items()) {
        // Загружаем колонки
        std::vector<Column> columns;
        for (const auto& col_json : table_json["columns"]) {
            Column col;
            col.name = col_json["name"];
            col.type = static_cast<DataType::Type>(col_json["type"].get<int>());
            col.size = col_json["size"];
            col.is_key = col_json["is_key"];
            col.is_unique = col_json["is_unique"];
            col.is_autoincrement = col_json["is_autoincrement"];

            if (col_json.contains("default_value")) {
                switch (col.type.getType()) {
                    case DataType::Type::INT32:
                        col.default_value = col_json["default_value"].get<int32_t>();
                        break;
                    case DataType::Type::BOOLEAN:
                        col.default_value = col_json["default_value"].get<bool>();
                        break;
                    case DataType::Type::STRING:
                        col.default_value = col_json["default_value"].get<std::string>();
                        break;
                    case DataType::Type::BYTES: {
                        std::string hex = col_json["default_value"];
                        std::vector<uint8_t> bytes;
                        for (size_t i = 2; i < hex.size(); i += 2) {
                            bytes.push_back(std::stoi(hex.substr(i, 2), nullptr, 16));
                        }
                        col.default_value = bytes;
                        break;
                    }
                }
            }

            columns.push_back(col);
        }

        // Создаем таблицу
        auto table = std::make_shared<Table>(table_name, columns);

        std::string auto_increment_column;
        for (const auto& col : columns) {
            if (col.is_autoincrement) {
                auto_increment_column = col.name;
                break;
            }
        }

        // Загружаем данные
        int32_t max_id = 0;
        for (const auto& row_json : table_json["rows"]) {
            std::vector<DataType::Value> row;
            for (const auto& col : columns) {
                if (row_json.contains(col.name)) {
                    switch (col.type.getType()) {
                        case DataType::Type::INT32:
                            row.push_back(row_json[col.name].get<int32_t>());
                            if (col.is_autoincrement) {
                                max_id = std::max(max_id, row_json[col.name].get<int32_t>());
                            }
                            break;
                        case DataType::Type::BOOLEAN:
                            row.push_back(row_json[col.name].get<bool>());
                            break;
                        case DataType::Type::STRING:
                            row.push_back(row_json[col.name].get<std::string>());
                            break;
                        case DataType::Type::BYTES: {
                            std::string hex = row_json[col.name];
                            std::vector<uint8_t> bytes;
                            for (size_t i = 2; i < hex.size(); i += 2) {
                                bytes.push_back(std::stoi(hex.substr(i, 2), nullptr, 16));
                            }
                            row.push_back(bytes);
                            break;
                        }
                    }
                } else {
                    row.push_back(std::monostate{});
                }
            }
            table->add_row(row);
        }

        if (!auto_increment_column.empty()) {
            table->update_auto_increment_value(max_id + 1);
        }

        tables[table_name] = table;
    }
}

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

    std::string keyword = to_lower(tokens[0]);
    if (keyword == "create") {
        if (tokens.size() > 1) {
            if (to_lower(tokens[1]) == "table") {
                keyword += " table";
            } else if (to_lower(tokens[2]) == "index") {
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

    return command->execute(*this, tokens, query);
}