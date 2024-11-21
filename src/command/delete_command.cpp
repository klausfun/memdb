#include <iostream>
#include <algorithm>
#include <regex>
#include "command.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../db.hpp"

Result DeleteCommand::execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) {
    std::cout << "Executing Delete command:\n" << std::endl;
    return Result{};
}

//Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) {
//    auto [success, data] = parseDeleteQuery(query);
//    if (!success) {
//        throw std::runtime_error("Invalid DELETE syntax");
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
//    // Удаляем строки, удовлетворяющие условию
//    size_t deleted = 0;
//    auto& rows = table->get_rows();
//    auto it = rows.begin();
//    while (it != rows.end()) {
//        if (evaluator.evaluate(rpn_tokens, *it, table->get_columns())) {
//            it = rows.erase(it);
//            deleted++;
//        } else {
//            ++it;
//        }
//    }
//
//    return Result{{{"deleted", deleted}}};
//}