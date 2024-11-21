#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

Result UpdateCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    std::cout << "Executing Update command:\n" << std::endl;
    return Result{};
}

//Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) {
//    auto [success, data] = parseUpdateQuery(query);
//    if (!success) {
//        throw std::runtime_error("Invalid UPDATE syntax");
//    }
//
//    auto table = db.getTable(data.table_name);
//    if (!table) {
//        throw std::runtime_error("Table not found");
//    }
//
//    RPNConverter converter;
//    auto rpn_tokens = converter.convert(tokenizeCondition(data.condition));
//    ConditionEvaluator evaluator;
//
//    // Обновляем строки, удовлетворяющие условию
//    size_t updated = 0;
//    for (auto& row : table->get_rows()) {
//        if (evaluator.evaluate(rpn_tokens, row, table->get_columns())) {
//            // Обновляем значения в строке
//            for (const auto& [col, value] : data.updates) {
//                size_t idx = findColumnIndex(col, table->get_columns());
//                row[idx] = value;
//            }
//            updated++;
//        }
//    }
//
//    return Result{{{"updated", updated}}};
//}