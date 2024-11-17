#include "row.h"
#include <stdexcept>

template <typename T>
T Row::get(const std::string& column_name) const {
    auto it = columns.find(column_name);
    if (it != columns.end()) {
        return std::get<T>(it->second);
    }
    throw std::out_of_range("Column not found");
}

void Row::set(const std::string& column_name, const Value& value) {
    columns[column_name] = value;
}