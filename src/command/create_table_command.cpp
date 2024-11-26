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
            R"(\s*(?:\{\s*((?:key|unique|autoincrement)(?:\s*,\s*(?:key|unique|autoincrement))*)\s*\})?\s*)"
            R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*)"
            R"((int32|bool|string(?:\s*\[\s*(\d+)\s*\])?|bytes(?:\s*\[\s*(\d+)\s*\])?)\s*)"
            R"((?:=\s*((?:true|false|\d+|"[^"]*"|0x[0-9A-Fa-f]+))\s*)?\s*)"
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, createTablePattern)) {
        return {false, {}};
    }

    std::string columnsStr = matches[2].str();
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
            return {false, {}};
        }

        Column column;

        if (columnMatches[1].matched) {
            std::string attrs = columnMatches[1].str();
            std::regex attr_splitter(R"(\s*,\s*)");
            std::sregex_token_iterator iter(attrs.begin(), attrs.end(), attr_splitter, -1);
            std::sregex_token_iterator end;

            for (; iter != end; ++iter) {
                std::string attr = iter->str();
                attr.erase(0, attr.find_first_not_of(" \t\n\r"));
                attr.erase(attr.find_last_not_of(" \t\n\r") + 1);

                if (attr == "key") column.is_key = true;
                else if (attr == "autoincrement") column.is_autoincrement = true;
                else if (attr == "unique") column.is_unique = true;
                else return {false, {}};
            }
        }

        column.name = columnMatches[2].str();

        for (const auto& existing : columns) {
            if (existing.name == column.name) {
                return {false, {}};
            }
        }

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
        else {
            return {false, {}};
        }

        if (column.is_autoincrement && column.type.getType() != DataType::Type::INT32) {
            return {false, {}};
        }

        if (columnMatches[6].matched) {
            if (column.is_autoincrement) {
                return {false, {}};
            }

            std::string defaultVal = columnMatches[6].str();
            try {
                switch (column.type.getType()) {
                    case DataType::Type::BOOLEAN:
                        if (defaultVal != "true" && defaultVal != "false") {
                            return {false, {}};
                        }
                        column.default_value = (defaultVal == "true");
                        break;

                    case DataType::Type::INT32:
                        column.default_value = std::stoi(defaultVal);
                        break;

                    case DataType::Type::STRING: {
                        if (defaultVal.size() < 2 || defaultVal.front() != '"' || defaultVal.back() != '"') {
                            return {false, {}};
                        }
                        std::string str = defaultVal.substr(1, defaultVal.length() - 2);
                        if (str.length() > column.size) {
                            return {false, {}};
                        }
                        column.default_value = defaultVal;
                        break;
                    }

                    case DataType::Type::BYTES: {
                        if (defaultVal.substr(0, 2) != "0x") {
                            return {false, {}};
                        }
                        std::string hex = defaultVal.substr(2);
                        hex.erase(std::remove_if(hex.begin(), hex.end(), ::isspace), hex.end());

                        if (!std::all_of(hex.begin(), hex.end(), [](char c) {
                            return (c >= '0' && c <= '9') || 
                                   (c >= 'a' && c <= 'f') || 
                                   (c >= 'A' && c <= 'F');
                        })) {
                            return {false, {}};
                        }

                        if (hex.length() > column.size) {
                            return {false, {}};
                        }

                        column.default_value = defaultVal;
                        break;
                    }
                }
            } catch (const std::exception& e) {
                return {false, {}};
            }
        }

        columns.push_back(column);
    }

    if (columns.empty()) {
        return {false, {}};
    }

    return {true, columns};
}

Result CreateTableCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    auto [success, columns] = parseCreateTableQuery(query);
    if (!success) {
        return Result("Invalid CREATE TABLE syntax");
    }

    std::string table_name = tokens[2];
    db.createTable(table_name, columns);

    return Result{};
}
