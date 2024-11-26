#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

std::pair<bool, InsertData> InsertCommand::parseInsertQuery(const std::string& query) {
    const std::regex insertPattern(
            R"(^\s*insert\s*\(\s*([\s\S]*?)\s*\)\s*to\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$)"
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, insertPattern)) {
        return {false, {}};
    }

    InsertData data;
    data.table_name = matches[2].str();
    std::string valuesStr = matches[1].str();

    std::vector<std::string> valueParts;
    std::string currentPart;
    bool inQuotes = false;
    int parentheses = 0;

    for (size_t i = 0; i < valuesStr.length(); ++i) {
        char c = valuesStr[i];

        if (c == '"' && (i == 0 || valuesStr[i-1] != '\\')) {
            inQuotes = !inQuotes;
            currentPart += c;
        }
        else if (c == '(' && !inQuotes) parentheses++;
        else if (c == ')' && !inQuotes) parentheses--;
        else if (c == ',' && !inQuotes && parentheses == 0) {
            valueParts.push_back(currentPart);
            currentPart.clear();
            continue;
        }
        else {
            currentPart += c;
        }
    }
    if (!currentPart.empty() || !valueParts.empty()) {
        valueParts.push_back(currentPart);
    }

    data.is_named_format = false;
    for (const auto& part : valueParts) {
        std::string trimmed = part;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

        if (!trimmed.empty()) {
            data.is_named_format = (trimmed.find('=') != std::string::npos);
            break;
        }
    }

    for (const auto& part : valueParts) {
        std::string trimmed = part;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

        if (data.is_named_format) {
            if (trimmed.empty()) {
                continue;
            }

            size_t equalsPos = trimmed.find('=');
            if (equalsPos == std::string::npos) {
                return {false, {}};
            }

            std::string name = trimmed.substr(0, equalsPos);
            std::string value = trimmed.substr(equalsPos + 1);

            name.erase(0, name.find_first_not_of(" \t\n\r"));
            name.erase(name.find_last_not_of(" \t\n\r") + 1);
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            value.erase(value.find_last_not_of(" \t\n\r") + 1);

            data.values.emplace_back(name, value);
        } else {
            if (trimmed.find('=') != std::string::npos) {
                throw std::runtime_error("Mixed format detected. Please use either named format (name = value) or simple format (value1, value2, value3)");
            }

            data.values.emplace_back("", trimmed);
        }
    }

    return {true, data};
}

DataType::Value InsertCommand::parseValue(const std::string& value_str, const DataType& type) {
    try {
        switch (type.getType()) {
            case DataType::Type::INT32: {
                return std::stoi(value_str);
            }

            case DataType::Type::BOOLEAN: {
                if (value_str == "true" || value_str == "1") return true;
                if (value_str == "false" || value_str == "0") return false;
                throw std::runtime_error("Invalid boolean value: " + value_str);
            }

            case DataType::Type::STRING: {
                if (value_str.size() >= 2 && value_str.front() == '"' && value_str.back() == '"') {
                    return value_str;
                }
                return "\"" + value_str + "\"";
            }

            case DataType::Type::BYTES: {
                if (value_str.substr(0, 2) != "0x") {
                    throw std::runtime_error("Bytes value must start with 0x");
                }
                std::string hex = value_str.substr(2);
                hex.erase(std::remove_if(hex.begin(), hex.end(), ::isspace), hex.end());

                if (!std::all_of(hex.begin(), hex.end(), [](char c) {
                    return (c >= '0' && c <= '9') || 
                           (c >= 'a' && c <= 'f') || 
                           (c >= 'A' && c <= 'F');
                })) {
                    throw std::runtime_error("Invalid hex characters");
                }

                return value_str;
            }

            default:
                throw std::runtime_error("Unsupported type");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse value '" + value_str + "': " + e.what());
    }
}

Result InsertCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    try {
        auto [success, data] = parseInsertQuery(query);
        if (!success) {
            return Result("Invalid INSERT syntax");
        }

        auto table = db.getTable(data.table_name);
        if (!table) {
            return Result("Table '" + data.table_name + "' not found");
        }

        const auto& columns = table->get_columns();
        std::unordered_map<std::string, DataType::Value> values;

        if (data.is_named_format) {
            for (const auto& [name, value_str] : data.values) {
                auto it = std::find_if(columns.begin(), columns.end(),
                                     [&name](const Column& col) { return col.name == name; });

                if (it == columns.end()) {
                    return Result("Column '" + name + "' not found");
                }

                if (!it->is_autoincrement) {
                    values[name] = parseValue(value_str, it->type);
                }
            }
        } else {
            size_t value_idx = 0;
            size_t col_idx = 0;

            while (col_idx < columns.size()) {
                const auto& current_column = columns[col_idx];
                std::string value_str;

                if (value_idx < data.values.size()) {
                    value_str = data.values[value_idx].second;
                }

                if (!current_column.is_autoincrement) {
                    if (!value_str.empty()) {
                        try {
                            values[current_column.name] = parseValue(value_str, current_column.type);
                        } catch (const std::exception& e) {
                            throw std::runtime_error("Error parsing value for column '" +
                                                     current_column.name + "': " + e.what());
                        }
                    } else if (!std::holds_alternative<std::monostate>(current_column.default_value)) {
                        values[current_column.name] = current_column.default_value;
                    } else {
                        throw std::runtime_error("Missing value for column '" + current_column.name +
                                                 "' and no default value specified");
                    }
                }

                value_idx++;
                col_idx++;
            }
        }

        for (const auto& col : columns) {
            if (col.is_autoincrement) {
                values[col.name] = table->get_auto_increment_value();
                table->increment_auto_increment();
            }
        }

        for (const auto& col : columns) {
            if (values.find(col.name) == values.end()) {
                if (col.is_autoincrement) {
                    continue;
                }
                if (!std::holds_alternative<std::monostate>(col.default_value)) {
                    values[col.name] = col.default_value;
                } else {
                    throw std::runtime_error("Missing value for column '" + col.name + "'");
                }
            }
        }

        for (const auto& col : columns) {
            if (col.is_unique && values.find(col.name) != values.end()) {
                for (const auto& row : table->get_rows()) {
                    size_t col_idx = std::distance(columns.begin(),
                                                   std::find_if(columns.begin(), columns.end(),
                                                                [&](const Column& c) { return c.name == col.name; }));

                    if (row[col_idx] == values[col.name]) {
                        throw std::runtime_error("Unique constraint violation for column '" + col.name + "'");
                    }
                }
            }
        }

        table->insert_row(values);
        return Result{};
    } catch (const std::runtime_error& e) {
        return Result(e.what());
    } catch (const std::exception& e) {
        return Result("Unexpected error: " + std::string(e.what()));
    }
}