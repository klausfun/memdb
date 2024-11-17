#pragma once
#include <unordered_map>
#include <string>
#include <variant>
#include <vector>

using Value = std::variant<int32_t, bool, std::string, std::vector<uint8_t>>;

class Row {
public:
    template <typename T>
    T get(const std::string& column_name) const;

    void set(const std::string& column_name, const Value& value);

private:
    std::unordered_map<std::string, Value> columns;
};