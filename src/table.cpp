#include "table.h"

void Table::insert_row(const Row& row) {
    rows.push_back(row);
}

const std::vector<Row>& Table::get_rows() const {
    return rows;
}