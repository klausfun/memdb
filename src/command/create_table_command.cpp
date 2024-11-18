#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

std::pair<bool, std::vector<Column>> CreateTableCommand::parseCreateTableQuery(const std::string& query) {
    const std::regex createTablePattern(
            R"(^\s*create\s+table\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(\s*([\s\S]*)\s*\)\s*$)"
    );

    const std::regex columnPattern(
            R"(\s*(?:\{\s*((?:key|unique|autoincrement)(?:\s*,\s*(?:key|unique|autoincrement))*)\s*\})?\s*)"  // изменили группу атрибутов
            R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*)"
            R"((int32|bool|string(?:\s*\[\s*(\d+)\s*\])?|bytes(?:\s*\[\s*(\d+)\s*\])?)\s*)"
            R"((?:=\s*(true|false|\d+|"[^"]*"|[^,\s)]+))?\s*)"
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, createTablePattern)) {
        std::cout << "Failed to match create table pattern\n";
        return {false, {}};
    }

    std::string columnsStr = matches[2];
    std::vector<Column> columns;

    std::vector<std::string> columnDefs;
    std::string currentColumn;
    int braceCount = 0;

    for (char c : columnsStr) {
        if (c == '{') braceCount++;
        else if (c == '}') braceCount--;

        if (c == ',' && braceCount == 0) {
            columnDefs.push_back(currentColumn);
            currentColumn.clear();
        } else {
            currentColumn += c;
        }
    }
    if (!currentColumn.empty()) {
        columnDefs.push_back(currentColumn);
    }

    for (const auto& columnDef : columnDefs) {
        std::smatch columnMatches;
        if (!std::regex_match(columnDef, columnMatches, columnPattern)) {
            std::cout << "Failed to match column pattern\n";
            return {false, {}};
        }

        Column column;

        if (columnMatches[1].matched) {
            std::string attrs = columnMatches[1].str();
            // Разбиваем строку атрибутов по запятым
            std::regex attr_splitter(R"(\s*,\s*)");
            std::sregex_token_iterator iter(attrs.begin(), attrs.end(), attr_splitter, -1);
            std::sregex_token_iterator end;

            // Проверяем каждый атрибут
            for (; iter != end; ++iter) {
                std::string attr = iter->str();
                // Удаляем лишние пробелы
                attr.erase(0, attr.find_first_not_of(" \t\n\r"));
                attr.erase(attr.find_last_not_of(" \t\n\r") + 1);

                if (attr == "key") column.is_key = true;
                else if (attr == "autoincrement") column.is_autoincrement = true;
                else if (attr == "unique") column.is_unique = true;
            }
        }

        column.name = columnMatches[2].str();

        std::string typeStr = columnMatches[3].str();
        if (typeStr == "int32") {
            column.type = DataType::Type::INT32;
        }
        else if (typeStr == "bool") {
            column.type = DataType::Type::BOOLEAN;
        }
        else if (typeStr.find("string") == 0) {
            column.type = DataType::Type::STRING;
            if (columnMatches[4].matched) {
                column.size = std::stoul(columnMatches[4].str());
            }
        }
        else if (typeStr.find("bytes") == 0) {
            column.type = DataType::Type::BYTES;
            if (columnMatches[5].matched) {
                column.size = std::stoul(columnMatches[5].str());
            }
        }

        if (columnMatches[6].matched) {
            std::string defaultVal = columnMatches[6].str();
            try {
                if (column.type.getType() == DataType::Type::BOOLEAN) {
                    column.default_value = defaultVal == "true";
                } else if (column.type.getType() == DataType::Type::INT32) {
                    column.default_value = std::stoi(defaultVal);
                } else if (column.type.getType() == DataType::Type::STRING) {
                    column.default_value = defaultVal;
                } else if (column.type.getType() == DataType::Type::BYTES) {
                    std::vector<uint8_t> bytes(defaultVal.begin(), defaultVal.end());
                    column.default_value = bytes;
                }
            } catch (const std::exception& e) {
                std::cout << "Invalid default value\n";
                return {false, {}};
            }
        }

        columns.push_back(column);
    }

    return {true, columns};
}

Result CreateTableCommand::execute(Database& db, const std::vector<Token>& tokens, const std::string& query) {
    std::cout << "Executing Create Table command:\n" << std::endl;

    auto [success, columns] = parseCreateTableQuery(query);
    if (!success) {
        throw std::runtime_error("Invalid CREATE TABLE syntax");
    }

    std::string table_name = tokens[2].value;
    db.createTable(table_name, columns);

    return Result{};
}
