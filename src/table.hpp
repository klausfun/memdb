#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "column.hpp"

class Table {
public:
    Table(const std::string& name, const std::vector<Column>& columns);
    
    // Основные операции с данными
    Table insert_row() const;
    Table update_rows() const;
    Table delete_rows() const;
    std::vector<std::vector<DataType::Value>> select_rows() const;
    Table create_index(const std::string& column_name) const;
    
    // Геттеры
    const std::string& get_name() const { return name_; }
    const std::vector<Column>& get_columns() const { return columns_; }
    const std::vector<std::vector<DataType::Value>>& get_rows() const { return rows_; }
//    bool has_index(const std::string& column_name) const;
    
    // Валидация
    bool validate_row(const std::unordered_map<std::string, DataType::Value>& values) const;
    bool column_exists(const std::string& name) const;

private:
    std::string name_;
    std::vector<Column> columns_;
    std::vector<std::vector<DataType::Value>> rows_;
//    std::unordered_map<std::string, Index> indexes_;
    int32_t auto_increment_value{1};
    
    size_t get_column_index(const std::string& name) const;
    DataType::Value get_default_value(const Column& column) const;
};