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

struct UpdateData {
    std::string table_name;
    std::vector<std::pair<std::string, std::string>> column_values;
    std::string condition;
};

struct DeleteData {
    std::string table_name;
    std::string condition;
};

class Command {
public:
    virtual ~Command() = default;
    virtual Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) = 0;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    size_t findColumnIndex(const std::string& name, const std::vector<Column>& columns) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == name) {
                return i;
            }
        }

        throw std::runtime_error("Column not found: " + name);
    }
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
private:
    std::pair<bool, UpdateData> parseUpdateQuery(const std::string& query);
};

class DeleteCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
private:
    std::pair<bool, DeleteData> parseDeleteQuery(const std::string& query);
};

class CreateIndexCommand : public Command {
public:
    Result execute(Database& db, const std::vector<std::string>& tokens, const std::string& query) override;
};
