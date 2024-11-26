#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"
#include "../rpn.hpp"

std::pair<bool, SelectData> SelectCommand::parseSelectQuery(const std::string& query) {
    const std::regex selectPattern(
            R"(select\s+(.*?)\s+from\s+(\w+)(?:\s+where\s+(.*))?)",
            std::regex::icase
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, selectPattern)) {
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
                    return {false, {}};
                }
                data.columns.push_back(current_column);
                current_column.clear();
                expect_comma = true;
            }
        }
        else {
            if (expect_comma && !std::isspace(c)) {
                return {false, {}};
            }
            current_column += c;
        }
    }

    if (!current_column.empty()) {
        current_column = trim(current_column);
        if (expect_comma) {
            return {false, {}};
        }
        data.columns.push_back(current_column);
    }

    if (data.columns.empty()) {
        return {false, {}};
    }

    data.table_name = trim(matches[2].str());

    if (matches[3].matched) {
        data.condition = trim(matches[3].str());
    }

    return {true, data};
}

Result SelectCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    try {
        auto [success, data] = parseSelectQuery(query);
        if (!success) {
            return Result("Invalid SELECT syntax");
        }

        auto table = db.getTable(data.table_name);
        if (!table) {
            return Result("Table '" + data.table_name + "' not found");
        }

        // Проверяем существование колонок
        for (const auto& col_name : data.columns) {
            if (col_name != "*" && !table->column_exists(col_name)) {
                return Result("Column not found: " + col_name);
            }
        }

        std::vector<std::vector<DataType::Value>> result_rows;
        
        // Если условие пустое, выбираем все строки
        if (data.condition.empty()) {
            for (const auto& row : table->get_rows()) {
                std::vector<DataType::Value> result_row;
                if (data.columns[0] == "*") {
                    result_row = row;
                } else {
                    for (const auto& col_name : data.columns) {
                        size_t idx = findColumnIndex(col_name, table->get_columns());
                        result_row.push_back(row[idx]);
                    }
                }
                result_rows.push_back(result_row);
            }
        } else {
            Tokenizer tokenizer;
            std::vector<std::string> condition_tokens = tokenizer.tokenize(data.condition);

            // Проверяем существование колонок в условии
            for (const auto& token : condition_tokens) {
                if (std::isalpha(token[0]) && token != "true" && token != "false" && 
                    !table->column_exists(token)) {
                    return Result("Column not found in condition: " + token);
                }
            }

            RPNConverter converter;
            std::vector<std::string> rpn_tokens = converter.convert(condition_tokens);
            RPNCalculator calculator;

            for (const auto& row : table->get_rows()) {
                bool include_row;
                try {
                    include_row = calculator.calculate(rpn_tokens, row, table->get_columns());
                } catch (const std::exception& e) {
                    return Result("Error evaluating condition: " + std::string(e.what()));
                }
                
                if (include_row) {
                    std::vector<DataType::Value> result_row;
                    if (data.columns[0] == "*") {
                        result_row = row;
                    } else {
                        for (const auto& col_name : data.columns) {
                            size_t idx = findColumnIndex(col_name, table->get_columns());
                            const auto& col = table->get_columns()[idx];
                            const auto& value = row[idx];
                            
                            if (std::holds_alternative<int32_t>(value) && col.type.getType() != DataType::Type::INT32) {
                                return Result("Type mismatch for column: " + col_name);
                            }
                            if (std::holds_alternative<bool>(value) && col.type.getType() != DataType::Type::BOOLEAN) {
                                return Result("Type mismatch for column: " + col_name);
                            }
                            if (std::holds_alternative<std::string>(value) && 
                                col.type.getType() != DataType::Type::STRING && 
                                col.type.getType() != DataType::Type::BYTES) {
                                return Result("Type mismatch for column: " + col_name);
                            }
                            
                            result_row.push_back(value);
                        }
                    }
                    result_rows.push_back(result_row);
                }
            }
        }

        return Result(result_rows);
    } catch (const std::runtime_error& e) {
        return Result(e.what());
    } catch (const std::exception& e) {
        return Result("Unexpected error: " + std::string(e.what()));
    }
}
