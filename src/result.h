#pragma once
#include <string>
#include "row.h"
#include <vector>
#include <iterator>

class Result {
public:
    void add_row(const Row& row);
    bool is_ok() const;

    using iterator = std::vector<Row>::iterator;
    using const_iterator = std::vector<Row>::const_iterator;

    iterator begin() { return rows.begin(); }
    iterator end() { return rows.end(); }
    const_iterator begin() const { return rows.begin(); }
    const_iterator end() const { return rows.end(); }

private:
    bool success = true;
    std::vector<Row> rows;
};
