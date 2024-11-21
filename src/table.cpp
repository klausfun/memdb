#include "table.hpp"
#include <stdexcept>
#include <algorithm>

Table::Table(const std::string& name, const std::vector<Column>& columns)
    : name_(name), columns_(columns) {}

void Table::insert_row(const std::unordered_map<std::string, DataType::Value>& values) {
    std::vector<DataType::Value> row;
    row.reserve(columns_.size());

    for (const auto& column : columns_) {
        auto it = values.find(column.name);

        if (it != values.end()) {
            row.push_back(it->second);
        } else if (column.is_autoincrement) {
            row.push_back(auto_increment_value++);
        } else if (!std::holds_alternative<std::monostate>(column.default_value)) {
            row.push_back(column.default_value);
        } else {
            row.push_back(std::monostate{});
        }
    }

    add_row(row);
}

void Table::update_value(size_t row_idx, size_t col_idx, const DataType::Value& value) {
    rows_[row_idx][col_idx] = value;
}

void Table::delete_rows() {
    rows_.clear();
}

void Table::set_rows(const std::vector<std::vector<DataType::Value>>& new_rows) {
    rows_ = new_rows;
}

std::vector<std::vector<DataType::Value>> Table::select_rows() const {
    return std::vector<std::vector<DataType::Value>>{};
}

Table Table::create_index(const std::string& column_name) const {
    return Table("", std::vector<Column>{});
}

//bool Table::has_index(const std::string& column_name) const {
//    return indexes_.find(column_name) != indexes_.end();
//}

bool Table::validate_row(const std::unordered_map<std::string, DataType::Value>& values) const {
    for (const auto& [name, value] : values) {
        if (!column_exists(name)) {
            return false;
        }
    }
    return true;
}

bool Table::column_exists(const std::string& name) const {
    return std::any_of(columns_.begin(), columns_.end(),
        [&name](const Column& col) { return col.name == name; });
}

size_t Table::get_column_index(const std::string& name) const {
    for (size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].name == name) {
            return i;
        }
    }
    throw std::runtime_error("Column not found: " + name);
}

DataType::Value Table::get_default_value(const Column& column) const {
    return DataType::Value{};
}