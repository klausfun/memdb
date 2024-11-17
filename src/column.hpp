#pragma once
#include "types.hpp"

struct Column {
    std::string name;
    DataType type;
    size_t size{0};  // для string[N] и bytes[N]
    bool is_unique{false};
    bool is_autoincrement{false};
    bool is_key{false};
    DataType::Value default_value;

    Column() : name(""), type(DataType::Type::INT32), size(0),
               is_key(false), is_unique(false), is_autoincrement(false) {}
};