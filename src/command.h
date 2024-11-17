#pragma once
#include <string>
#include <vector>
#include "result.h"

class Database;
class Token;

class Command {
public:
    virtual ~Command() = default;
    virtual Result execute(Database& db, const std::vector<Token>& tokens) = 0;
};

class CreateTableCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};

class InsertCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};

class SelectCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};

class UpdateCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};

class DeleteCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};

class CreateIndexCommand : public Command {
public:
    Result execute(Database& db, const std::vector<Token>& tokens) override;
};
