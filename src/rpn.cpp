#include <iostream>
#include "rpn.hpp"
#include "parser.hpp"

bool isOperator(const std::string& token) {
    static const std::set<std::string> operators = {
            "+", "-", "*", "/", "%",
            "<", ">", "<=", ">=", "=", "!=",
            "&&", "||", "^^", "!", "|"
    };
    return operators.find(token) != operators.end();
}

std::vector<std::string> RPNConverter::mergeOperators(const std::vector<std::string>& tokens) {
    std::vector<std::string> result;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i + 1 < tokens.size() && isDoubleOperator(tokens[i], tokens[i + 1])) {
            result.push_back(tokens[i] + tokens[i + 1]);
            i++;
        } else {
            result.push_back(tokens[i]);
        }
    }

    return result;
}

bool RPNConverter::isDoubleOperator(const std::string& first, const std::string& second) {
    if (first == "|" && second == "|") return true;
    if (first == "&" && second == "&") return true;
    if (first == "<" && second == "=") return true;
    if (first == ">" && second == "=") return true;
    if (first == "!" && second == "=") return true;
    if (first == "^" && second == "^") return true;
    return false;
}

int RPNConverter::getOperatorPriority(const std::string& op) {
    if (op == "|") return 8;
    if (op == "!") return 7;
    if (op == "*" || op == "/" || op == "%") return 6;
    if (op == "+" || op == "-") return 5;
    if (op == "<" || op == ">" || op == "<=" || op == ">=" ||
        op == "=" || op == "==" || op == "!=") return 4;
    if (op == "&&") return 3;
    if (op == "^^") return 2;
    if (op == "||") return 1;
    return 0;
}

std::vector<std::string> RPNConverter::convert(const std::vector<std::string>& tokens) {
    std::vector<std::string> output;
    std::stack<std::string> operators;

    std::vector<std::string> processed_tokens = mergeOperators(tokens);

    bool flag = false;
    for (const auto& token : processed_tokens) {
        if (isOperator(token)) {
            while (!operators.empty() && operators.top() != "(" &&
                   getOperatorPriority(operators.top()) >= getOperatorPriority(token)) {
                output.push_back(operators.top());
                operators.pop();
                if (token == "|") {
                    flag = true;
                }
            }

            if (flag) {
                flag = false;
                continue;
            }
            operators.push(token);
        } else if (token == "(") {
            operators.push(token);
        } else if (token == ")") {
            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }

            if (!operators.empty()) {
                operators.pop();
            }
        } else {
            output.push_back(token);
        }
    }

    while (!operators.empty()) {
        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}

bool RPNCalculator::isNumber(const std::string& token) {
    if (token.empty()) return false;

    size_t start = 0;
    if (token[0] == '-' || token[0] == '+') {
        if (token.length() == 1) return false;
        start = 1;
    }

    for (size_t i = start; i < token.length(); i++) {
        if (!std::isdigit(token[i])) {
            return false;
        }
    }

    return true;
}

size_t RPNCalculator::findColumnIndex(const std::string& name, const std::vector<Column>& columns) {
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == name) {
            return i;
        }
    }

    throw std::runtime_error("Column not found: " + name);
}

DataType::Value RPNCalculator::calculateUnary(const std::string& op, const DataType::Value& val) {
    if (op == "!") {
        if (!std::holds_alternative<bool>(val)) {
            throw std::runtime_error("! operator requires boolean operand");
        }

        return !std::get<bool>(val);
    }

    if (op == "|") {
        if (std::holds_alternative<std::string>(val)) {
            return static_cast<int32_t>(std::get<std::string>(val).length());
        }

        if (std::holds_alternative<std::vector<uint8_t>>(val)) {
            return static_cast<int32_t>(std::get<std::vector<uint8_t>>(val).size());
        }

        std::cerr << "Current type index: " << std::get<int32_t>(val) << std::endl;
        std::cerr << "Current type index: " << val.index() << std::endl;
        std::cerr << "Contains string: " << std::holds_alternative<std::string>(val) << std::endl;
        std::cerr << "Contains bytes: " << std::holds_alternative<std::vector<uint8_t>>(val) << std::endl;
        std::cerr << "Contains int: " << std::holds_alternative<int32_t>(val) << std::endl;
        std::cerr << "Contains bool: " << std::holds_alternative<bool>(val) << std::endl;

        throw std::runtime_error("| operator requires string or bytes operand");
    }

    throw std::runtime_error("Unknown unary operator: " + op);
}

DataType::Value RPNCalculator::calculateBinary(const std::string &op, const DataType::Value &left,
                                               const DataType::Value &right) {
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (!std::holds_alternative<int32_t>(left) || !std::holds_alternative<int32_t>(right)) {
            throw std::runtime_error("Arithmetic operations require numeric operands");
        }

        int32_t l = std::get<int32_t>(left);
        int32_t r = std::get<int32_t>(right);

        if (op == "+") return l + r;
        if (op == "-") return l - r;
        if (op == "*") return l * r;
        if (op == "/") {
            if (r == 0) throw std::runtime_error("Division by zero");
            return l / r;
        }
        if (op == "%") {
            if (r == 0) throw std::runtime_error("Modulo by zero");
            return l % r;
        }

        throw std::runtime_error("Unknown arithmetic operator: " + op);
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "=" || op == "!=") {
        if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
            int32_t l = std::get<int32_t>(left);
            int32_t r = std::get<int32_t>(right);

            if (op == "<") return l < r;
            if (op == ">") return l > r;
            if (op == "<=") return l <= r;
            if (op == ">=") return l >= r;
            if (op == "=") return l == r;
            if (op == "!=") return l != r;

        } else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool l = std::get<bool>(left);
            bool r = std::get<bool>(right);

            if (op == "<") return l < r;
            if (op == ">") return l > r;
            if (op == "<=") return l <= r;
            if (op == ">=") return l >= r;
            if (op == "=") return l == r;
            if (op == "!=") return l != r;

        } else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
            const std::string& l = std::get<std::string>(left);
            const std::string& r = std::get<std::string>(right);

            if (op == "<") return l < r;
            if (op == ">") return l > r;
            if (op == "<=") return l <= r;
            if (op == ">=") return l >= r;
            if (op == "=") return l == r;
            if (op == "!=") return l != r;

        } else if ( std::holds_alternative<std::vector<uint8_t>>(left) &&
                    std::holds_alternative<std::vector<uint8_t>>(right)) {
            const auto& l = std::get<std::vector<uint8_t>>(left);
            const auto& r = std::get<std::vector<uint8_t>>(right);

            if (op == "<") return l < r;
            if (op == ">") return l > r;
            if (op == "<=") return l <= r;
            if (op == ">=") return l >= r;
            if (op == "=") return l == r;
            if (op == "!=") return l != r;
        }

        throw std::runtime_error("Type mismatch in comparison operation");
    }

    if (op == "&&" || op == "||" || op == "^^") {
        if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool l = std::get<bool>(left);
            bool r = std::get<bool>(right);

            if (op == "&&") return l && r;
            if (op == "||") return l || r;
            if (op == "^^") return l ^ r;
        }

        if (op == "^^" && std::holds_alternative<int32_t>(left) &&
            std::holds_alternative<int32_t>(right)) {
            return std::get<int32_t>(left) ^ std::get<int32_t>(right);
        }

        throw std::runtime_error("Logical operations require boolean operands");
    }

    throw std::runtime_error("Unknown binary operator: " + op);
}

void printStack(const std::stack<DataType::Value>& stack) {
    // Создаем копию стека, так как оригинальный будет уничтожен при выводе
    auto temp_stack = stack;

    std::cerr << "Stack (top to bottom): ";
    while (!temp_stack.empty()) {
        const auto& val = temp_stack.top();

        // Выводим значение в зависимости от типа
        if (std::holds_alternative<int32_t>(val)) {
            std::cerr << std::get<int32_t>(val);
        }
        else if (std::holds_alternative<bool>(val)) {
            std::cerr << (std::get<bool>(val) ? "true" : "false");
        }
        else if (std::holds_alternative<std::string>(val)) {
            std::cerr << "'" << std::get<std::string>(val) << "'";
        }
        else if (std::holds_alternative<std::vector<uint8_t>>(val)) {
            std::cerr << "bytes[" << std::get<std::vector<uint8_t>>(val).size() << "]";
        }

        temp_stack.pop();
        if (!temp_stack.empty()) {
            std::cerr << " | ";
        }
    }
    std::cerr << std::endl;
}

bool RPNCalculator::calculate(const std::vector<std::string>& rpn_tokens,
               const std::vector<DataType::Value>& row,
               const std::vector<Column>& columns) {
    std::stack<DataType::Value> stack;

    for (const auto& token : rpn_tokens) {
        if (isOperator(token)) {
            if (token == "|" || token == "!") {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid expression: not enough operands");
                }

                auto operand = stack.top();
                stack.pop();
                stack.push(calculateUnary(token, operand));
            } else {
                if (stack.size() < 2) {
                    throw std::runtime_error("Invalid expression: not enough operands");
                }

                auto right = stack.top();
                stack.pop();
                auto left = stack.top();
                stack.pop();
                stack.push(calculateBinary(token, left, right));
            }
        } else if (isNumber(token)) {
            stack.push(std::stoi(token));
        }
        else if (token == "true" || token == "false") {
            stack.push(token == "true");
        }
        else {
            try {
                size_t idx = findColumnIndex(token, columns);
                stack.push(row[idx]);
            } catch (const std::runtime_error&) {
                stack.push(token);
            }
        }
    }

    if (stack.empty()) {
        throw std::runtime_error("Invalid expression: empty result");
    }

    const auto& result = stack.top();

    if (!std::holds_alternative<bool>(result)) {
        throw std::runtime_error("Expression must evaluate to boolean value");
    }

    return std::get<bool>(result);;
}