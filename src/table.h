#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <vector>
#include "row.h"

class Table {
public:
    Table(std::string query);

    void insert(std::string query);
    void create(std::string query);

    void insert_row(const Row& row);

    const std::vector<Row>& get_rows() const;

private:
    std::vector<Row> rows;
};
