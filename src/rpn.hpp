#pragma once
#include <string>
#include <vector>
#include <stack>
#include <set>
#include "types.hpp"
#include "column.hpp"

class RPNConverter {
public:
    std::vector<std::string> convert(const std::vector<std::string>& tokens);

private:
    std::vector<std::string> mergeOperators(const std::vector<std::string>& tokens);

    bool isDoubleOperator(const std::string& first, const std::string& second);

    int getOperatorPriority(const std::string& op);
};

class RPNCalculator {
public:
    bool calculate(const std::vector<std::string>& rpn_tokens,
                  const std::vector<DataType::Value>& row,
                  const std::vector<Column>& columns);

    DataType::Value calculate_value(const std::vector<std::string>& rpn_tokens,
                                  const std::vector<DataType::Value>& row,
                                  const std::vector<Column>& columns);

private:
    bool isNumber(const std::string& token);
    DataType::Value calculateUnary(const std::string& op, const DataType::Value& val);
    DataType::Value calculateBinary(const std::string& op, const DataType::Value& left, const DataType::Value& right);
    size_t findColumnIndex(const std::string& name, const std::vector<Column>& columns);
};