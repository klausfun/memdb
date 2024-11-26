#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"
#include "../rpn.hpp"

std::pair<bool, UpdateData> UpdateCommand::parseUpdateQuery(const std::string& query) {
    const std::regex updatePattern(
            R"(update\s+(\w+)\s+set\s+(.*?)(?:\s+where\s+(.*))?$)",
            std::regex::icase
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, updatePattern)) {
        std::cerr << "Failed to match update pattern\n";
        return {false, {}};
    }

    UpdateData data;
    data.table_name = trim(matches[1].str());

    std::string set_expr = matches[2].str();

    std::regex assign_pattern(R"((\w+)\s*=\s*([^,]+))");
    std::string::const_iterator searchStart(set_expr.cbegin());
    std::smatch assign_matches;

    while (std::regex_search(searchStart, set_expr.cend(), assign_matches, assign_pattern)) {
        std::string column = trim(assign_matches[1].str());
        std::string value = trim(assign_matches[2].str());
        data.column_values.emplace_back(column, value);
        searchStart = assign_matches.suffix().first;
    }

    if (data.column_values.empty()) {
        return {false, {}};
    }

    if (matches[3].matched) {
        data.condition = trim(matches[3].str());
    }

    return {true, data};
}

Result UpdateCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    try {
        auto [success, data] = parseUpdateQuery(query);
        if (!success) {
            return Result("Invalid UPDATE syntax");
        }

        auto table = db.getTable(data.table_name);
        if (!table) {
            return Result("Table '" + data.table_name + "' not found");
        }

        Tokenizer tokenizer;
        std::vector<std::string> condition_tokens = tokenizer.tokenize(data.condition);

        RPNConverter converter;
        std::vector<std::string> rpn_tokens = converter.convert(condition_tokens);

        RPNCalculator calculator;

        size_t updated = 0;
        size_t row_idx = 0;
        for (auto& row : table->get_rows()) {
            if (calculator.calculate(rpn_tokens, row, table->get_columns())) {
                for (const auto& [col, value_str] : data.column_values) {
                    size_t col_idx = findColumnIndex(col, table->get_columns());
                    const auto& column = table->get_columns()[col_idx];

                    DataType::Value new_value;

                    if (value_str.find(col) != std::string::npos) {

                        auto current_value = row[col_idx];

                        std::string modified_expr = value_str;
                        if (std::holds_alternative<int32_t>(current_value)) {
                            modified_expr = std::regex_replace(modified_expr, std::regex(col),
                                std::to_string(std::get<int32_t>(current_value)));
                        } else if (std::holds_alternative<std::string>(current_value)) {
                            modified_expr = std::regex_replace(modified_expr, std::regex(col),
                                std::get<std::string>(current_value));
                        }

                        std::vector<std::string> expr_tokens = tokenizer.tokenize(modified_expr);
                        std::vector<std::string> rpn_expr = converter.convert(expr_tokens);

                        new_value = calculator.calculate_value(rpn_expr, row, table->get_columns());
                    } else {
                        if (column.type.getType() == DataType::Type::INT32) {
                            new_value = std::stoi(value_str);
                        } else if (column.type.getType() == DataType::Type::BOOLEAN) {
                            new_value = (value_str == "true");
                        } else if (column.type.getType() == DataType::Type::STRING) {
                            if (value_str.front() == '\"' && value_str.back() == '\"') {
                                new_value = value_str;
                            } else {
                                new_value = "\"" + value_str + "\"";
                            }
                        } else if (column.type.getType() == DataType::Type::BYTES) {
                            if (value_str.substr(0, 2) != "0x") {
                                return Result("Invalid bytes format");
                            }
                            std::string hex = value_str.substr(2);
                            if (hex.length() % 2 != 0) {
                                return Result("Invalid hex string length");
                            }
                            if (hex.length() / 2 > column.size) {
                                return Result("Bytes value too long");
                            }
                            if (!std::all_of(hex.begin(), hex.end(), [](char c) {
                                return (c >= '0' && c <= '9') ||
                                       (c >= 'a' && c <= 'f') ||
                                       (c >= 'A' && c <= 'F');
                            })) {
                                return Result("Invalid hex characters");
                            }
                            new_value = value_str;
                        }
                    }

                    table->update_value(row_idx, col_idx, new_value);
                }
                updated++;
            }
            row_idx++;
        }

        return Result{};
    } catch (const std::runtime_error& e) {
        return Result(e.what());
    } catch (const std::exception& e) {
        return Result("Unexpected error: " + std::string(e.what()));
    }
}