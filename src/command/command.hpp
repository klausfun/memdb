#pragma once
#include <string>
#include <vector>
#include "../result.hpp"
#include "../column.hpp"

class Database;

struct InsertData {
    std::string table_name;
    std::vector<std::pair<std::string, std::string>> values;
    bool is_named_format;
};

struct SelectData {
    std::string table_name;
    std::vector<std::string> columns;
    std::string condition;
};

class Command {
public:
    virtual ~Command() = default;
    virtual Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) = 0;
};

class CreateTableCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
private:
    std::pair<bool, std::vector<Column>> parseCreateTableQuery(const std::string& query);
//    std::vector<Column> parseColumns(const std::vector<Token>& tokens);
//    bool isValidCreateTableQuery(const std::string& query);
};

class InsertCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
private:
    std::pair<bool, InsertData> parseInsertQuery(const std::string& query);
    DataType::Value parseValue(const std::string& value_str, const DataType& type);
};

class SelectCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
private:
    std::pair<bool, SelectData> parseSelectQuery(const std::string& query);
};

class UpdateCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
};

class DeleteCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
};

class CreateIndexCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
};
