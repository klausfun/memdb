#pragma once
#include <string>
#include <vector>
#include "../result.hpp"
#include "../column.hpp"

class Database;
class Token;

struct InsertData {
    std::string table_name;
    std::vector<std::pair<std::string, std::string>> values;
    bool is_named_format;
};

class Command {
public:
    virtual ~Command() = default;
    virtual Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) = 0;
};

class CreateTableCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
private:
    std::pair<bool, std::vector<Column>> parseCreateTableQuery(const std::string& query);
//    std::vector<Column> parseColumns(const std::vector<Token>& tokens);
//    bool isValidCreateTableQuery(const std::string& query);
};

class InsertCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
private:
    std::pair<bool, InsertData> parseInsertQuery(const std::string& query);
    DataType::Value parseValue(const std::string& value_str, const DataType& type);
    std::pair<bool, InsertData> parseNamedInsert(const std::string& table_name, const std::string& values_str);
    std::pair<bool, InsertData> parseSimpleInsert(const std::string& table_name, const std::string& values_str);

};

class SelectCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
};

class UpdateCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
};

class DeleteCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
};

class CreateIndexCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens, const std::string& query) override;
};
