#include "table.hpp"
#include <stdexcept>
#include <algorithm>

Table::Table(const std::string& name, const std::vector<Column>& columns)
    : name_(name), columns_(columns) {}

void Table::insert_row(const std::unordered_map<std::string, DataType::Value>& values) {
    std::vector<DataType::Value> row;
    row.reserve(columns_.size());

    // Проходим по колонкам в правильном порядке
    for (const auto& column : columns_) {
        auto it = values.find(column.name);

        if (it != values.end()) {
            // Используем предоставленное значение
            row.push_back(it->second);
        }
        else if (column.is_autoincrement) {
            // Для autoincrement используем следующее значение
            row.push_back(auto_increment_value++);
        }
        else if (!std::holds_alternative<std::monostate>(column.default_value)) {
            // Используем значение по умолчанию
            row.push_back(column.default_value);
        }
        else {
            // Если нет значения и нет значения по умолчанию, используем null
            row.push_back(std::monostate{});
        }
    }

    // Добавляем строку, используя существующий метод
    add_row(row);
}

Table Table::update_rows() const {
    return Table("", std::vector<Column>{});
}

Table Table::delete_rows() const {
    return Table("", std::vector<Column>{});
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

DataType::Value Table::get_default_value(const Column& column) const {
    return DataType::Value{};
}