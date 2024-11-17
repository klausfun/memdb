#include <iostream>
#include "command.hpp"
#include "parser.hpp"

Result CreateTableCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Create Table command:\n" << std::endl;
    std::vector<Column> columns = parseColumns(tokens);
    for (const auto& column : columns) {
        std::cout << "name: " << column.name << ", type: " << column.type.typeToString(column.type.getType()) <<
        ", size: " << column.size << ", is_unique: " << column.is_unique <<
       ", is_autoincrement: " << column.is_autoincrement << ", is_key: " << column.is_key <<
      ", default_value: ";

        std::visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::cout << "null";
            } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                std::cout << "<bytes>";
            } else {
                std::cout << value;
            }
        }, column.default_value);

        std::cout << std::endl;
    }
    return Result{};
}

std::vector<Column> CreateTableCommand::parseColumns(const std::vector<Token>& tokens) {
    std::vector<Column> columns;
    Column current;
    int pos = 4;

    while (pos < tokens.size() && tokens[pos].value != ")") {
        // Парсинг атрибутов
        if (tokens[pos].value == "{") {
            pos++; // пропускаем {
            while (tokens[pos].value != "}") {
                if (tokens[pos].value == "key") current.is_key = true;
                else if (tokens[pos].value == "autoincrement") current.is_autoincrement = true;
                else if (tokens[pos].value == "unique") current.is_unique = true;

                pos++;
                if (tokens[pos].value == ",") pos++; // пропускаем запятую между атрибутами
            }
            pos++; // пропускаем }
        }

        // Имя колонки
        current.name = tokens[pos].value;
        pos += 2; // пропускаем имя и ':'

        // Тип
        std::string typeStr = tokens[pos].value;
        if (typeStr == "int32") current.type = DataType::Type::INT32;
        else if (typeStr == "string") current.type = DataType::Type::STRING;
        else if (typeStr == "bytes") current.type = DataType::Type::BYTES;
        else if (typeStr == "bool") current.type = DataType::Type::BOOLEAN;
        pos++;

        // Размер для string и bytes
        if (pos < tokens.size() && tokens[pos].value == "[") {
            pos++;
            current.size = std::stoul(tokens[pos].value);
            pos += 2; // пропускаем размер и ']'
        }

        // Значение по умолчанию
        if (pos < tokens.size() && tokens[pos].value == "=") {
            pos++;
            std::string defaultVal = tokens[pos].value;
            if (current.type.getType() == DataType::Type::BOOLEAN) {
                current.default_value = defaultVal == "true";
            } else if (current.type.getType() == DataType::Type::INT32) {
                current.default_value = std::stoi(defaultVal);
            } else if (current.type.getType() == DataType::Type::STRING) {
                current.default_value = defaultVal;
            } else if (current.type.getType() == DataType::Type::BYTES) {
                std::vector<uint8_t> bytes(defaultVal.begin(), defaultVal.end());
                current.default_value = bytes;
            }
            pos++;
        }

        columns.push_back(current);
        current = Column{};

        if (pos < tokens.size() && tokens[pos].value == ",") pos++;
    }

    return columns;
};

Result InsertCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing INSERT command:\n" << std::endl;
    return Result{};
}

Result SelectCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing SELECT command:\n" << std::endl;
    return Result{};
}

Result UpdateCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Update command:\n" << std::endl;
    return Result{};
}

Result DeleteCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Delete command:\n" << std::endl;
    return Result{};
}

Result CreateIndexCommand::execute(Database& db, const std::vector<Token>& tokens) {
    std::cout << "Executing Create Index command:\n" << std::endl;
    return Result{};
}