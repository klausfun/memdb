#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"
#include "../rpn.hpp"

std::pair<bool, DeleteData> DeleteCommand::parseDeleteQuery(const std::string& query) {
    const std::regex deletePattern(
            R"(delete\s+(?:from\s+)?(\w+)(?:\s+where\s+(.*))?$)",
            std::regex::icase
    );

    std::smatch matches;
    if (!std::regex_match(query, matches, deletePattern)) {
        std::cerr << "Failed to match delete pattern\n";
        return {false, {}};
    }

    DeleteData data;
    data.table_name = trim(matches[1].str());

    if (matches[2].matched) {
        data.condition = trim(matches[2].str());
    }

    return {true, data};
}

Result DeleteCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    auto [success, data] = parseDeleteQuery(query);
    if (!success) {
        return Result("Invalid DELETE syntax");
    }

    auto table = db.getTable(data.table_name);
    if (!table) {
        return Result("Table not found: " + data.table_name);
    }

    size_t deleted = 0;
    std::vector<std::vector<DataType::Value>> new_rows;

    if (data.condition.empty()) {
        deleted = table->get_rows().size();
        table->delete_rows();
    } else {
        Tokenizer tokenizer;
        std::vector<std::string> condition_tokens = tokenizer.tokenize(data.condition);

        RPNConverter converter;
        std::vector<std::string> rpn_tokens = converter.convert(condition_tokens);

        RPNCalculator calculator;

        for (const auto& row : table->get_rows()) {
            if (!calculator.calculate(rpn_tokens, row, table->get_columns())) {
                new_rows.push_back(row);
            } else {
                deleted++;
            }
        }

        table->set_rows(new_rows);
    }

    return Result{};
}