#include "table.hpp"
#include <stdexcept>
#include <algorithm>

Table::Table(const std::string& name, const std::vector<Column>& columns)
    : name_(name), columns_(columns) {}

Table Table::insert_row() const {}

Table Table::update_rows() const {}

Table Table::delete_rows() const {}

std::vector<std::vector<DataType::Value>> Table::select_rows() const {}

Table Table::create_index(const std::string& column_name) const {}

//bool Table::has_index(const std::string& column_name) const {
//    return indexes_.find(column_name) != indexes_.end();
//}

bool Table::validate_row(const std::unordered_map<std::string, DataType::Value>& values) const {
    for (const auto& [name, value] : values) {
        if (!column_exists(name)) {
            return false;
        }
        // Здесь можно добавить дополнительные проверки типов данных
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

DataType::Value Table::get_default_value(const Column& column) const {}