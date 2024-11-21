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
        std::cout << "Failed to match insert pattern\n";
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
    if (!currentPart.empty()) {
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

        if (trimmed.empty()) continue;

        if (data.is_named_format) {
            size_t equalsPos = trimmed.find('=');
            if (equalsPos == std::string::npos) {
                std::cout << "Expected '=' in named format: " << trimmed << "\n";
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
        std::cout << "Parsing value: '" << value_str << "' as type: "
                  << static_cast<int>(type.getType()) << std::endl;

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
                    return value_str.substr(1, value_str.size() - 2);
                }
                return value_str;
            }

            case DataType::Type::BYTES: {
                std::cout << "Parsing bytes value: '" << value_str << "'\n";
                if (value_str.substr(0, 2) != "0x") {
                    throw std::runtime_error("Bytes value must start with 0x");
                }
                std::vector<uint8_t> bytes;
                for (size_t i = 2; i < value_str.size(); i += 2) {
                    bytes.push_back(std::stoi(value_str.substr(i, 2), nullptr, 16));
                }
                return bytes;
            }

            default:
                throw std::runtime_error("Unsupported type");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse value '" + value_str + "': " + e.what());
    }
}

Result InsertCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    std::cout << "Executing Insert command:\n";

    auto [success, data] = parseInsertQuery(query);
    if (!success) {
        throw std::runtime_error("Invalid INSERT syntax");
    }

    auto table = db.getTable(data.table_name);
    if (!table) {
        throw std::runtime_error("Table '" + data.table_name + "' not found");
    }

    const auto& columns = table->get_columns();
    std::unordered_map<std::string, DataType::Value> values;

    if (data.is_named_format) {
        for (const auto& [name, value_str] : data.values) {
            auto it = std::find_if(columns.begin(), columns.end(),
                                   [&name](const Column& col) { return col.name == name; });

            if (it == columns.end()) {
                throw std::runtime_error("Column '" + name + "' not found");
            }

            if (!it->is_autoincrement) {
                values[name] = parseValue(value_str, it->type);
            }
        }
    } else {
        size_t value_idx = 0;
        size_t col_idx = 0;

        while (col_idx < columns.size() && columns[col_idx].is_autoincrement) {
            col_idx++;
        }

        while (value_idx < data.values.size() && col_idx < columns.size()) {
            const auto& value_str = data.values[value_idx].second;
            if (!value_str.empty()) {
                values[columns[col_idx].name] = parseValue(value_str, columns[col_idx].type);
            }
            value_idx++;

            do {
                col_idx++;
            } while (col_idx < columns.size() && columns[col_idx].is_autoincrement);
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
}