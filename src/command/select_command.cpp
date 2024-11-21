#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"
#include "../rpn.hpp"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::pair<bool, SelectData> SelectCommand::parseSelectQuery(const std::string& query) {
    const std::regex selectPattern(
            R"(select\s+(.*?)\s+from\s+(\w+)(?:\s+where\s+(.*))?)",
            std::regex::icase
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, selectPattern)) {
        std::cout << "Failed to match select pattern\n";
        return {false, {}};
    }

    SelectData data;

    std::string columns_str = matches[1].str();
    std::string current_column;
    bool in_quotes = false;
    bool expect_comma = false;

    for (size_t i = 0; i < columns_str.length(); ++i) {
        char c = columns_str[i];

        if (c == '"') {
            in_quotes = !in_quotes;
            current_column += c;
        }
        else if (c == ',' && !in_quotes) {
            if (current_column.empty()) {
                std::cout << "Empty column name\n";
                return {false, {}};
            }

            current_column = trim(current_column);
            data.columns.push_back(current_column);
            current_column.clear();
            expect_comma = false;
        }
        else if (std::isspace(c) && !in_quotes) {
            if (!current_column.empty()) {
                current_column = trim(current_column);
                if (expect_comma) {
                    std::cout << "Expected comma between columns\n";
                    return {false, {}};
                }
                data.columns.push_back(current_column);
                current_column.clear();
                expect_comma = true;
            }
        }
        else {
            if (expect_comma && !std::isspace(c)) {
                std::cout << "Expected comma between columns\n";
                return {false, {}};
            }
            current_column += c;
        }
    }

    if (!current_column.empty()) {
        current_column = trim(current_column);
        if (expect_comma) {
            std::cout << "Expected comma between columns\n";
            return {false, {}};
        }
        data.columns.push_back(current_column);
    }

    if (data.columns.empty()) {
        std::cout << "No columns specified\n";
        return {false, {}};
    }

    data.table_name = trim(matches[2].str());

    if (matches[3].matched) {
        data.condition = trim(matches[3].str());
    }

    return {true, data};
}

size_t findColumnIndex(const std::string& name, const std::vector<Column>& columns) {
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == name) {
            return i;
        }
    }

    throw std::runtime_error("Column not found: " + name);
}

Result SelectCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    std::cout << "Executing SELECT command:\n";

    auto [success, data] = parseSelectQuery(query);
    if (!success) {
        throw std::runtime_error("Invalid SELECT syntax");
    }

    auto table = db.getTable(data.table_name);
    if (!table) {
        throw std::runtime_error("Table '" + data.table_name + "' not found");
    }

    Tokenizer tokenizer;
    std::vector<std::string> condition_tokens = tokenizer.tokenize(data.condition);

    RPNConverter converter;
    std::vector<std::string> rpn_tokens = converter.convert(condition_tokens);

    RPNCalculator calculator;

    std::vector<std::vector<DataType::Value>> result_rows;
    for (const auto& row : table->get_rows()) {
        bool include_row = true;

        if (!rpn_tokens.empty()) {
            include_row = calculator.calculate(rpn_tokens, row, table->get_columns());
        }

        if (include_row) {
            std::vector<DataType::Value> result_row;
            for (const auto& col_name : data.columns) {
                size_t idx = findColumnIndex(col_name, table->get_columns());
                result_row.push_back(row[idx]);
            }
            result_rows.push_back(result_row);
        }
    }

    return Result{result_rows};
}
