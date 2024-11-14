#pragma once
#include <string>
#include "result.h"

class Database;

class Command {
public:
    virtual ~Command() = default;
    virtual Result execute(Database& db, const std::string& query) = 0;
};

class CreateCommand : public Command {
public:
    Result execute(Database& db, const std::string& query) override;
};

class InsertCommand : public Command {
public:
    Result execute(Database& db, const std::string& query) override;
};

class SelectCommand : public Command {
public:
    Result execute(Database& db, const std::string& query) override;
};

