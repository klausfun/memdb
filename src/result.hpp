#pragma once
#include <string>
#include <vector>
#include <iterator>
#include "types.hpp"

class Result {
public:
    Result () = default;

    explicit Result(std::vector<std::vector<DataType::Value>> result_rows)
            : success(true)
            , rows(std::move(result_rows))
    {}

    explicit Result(const std::string& error_message)
            : success(false)
            , error(error_message)
    {}

    bool is_ok() const;
    std::string get_error();

    using iterator = std::vector<std::vector<DataType::Value>>::iterator;
    using const_iterator = std::vector<std::vector<DataType::Value>>::const_iterator;

    iterator begin() { return rows.begin(); }
    iterator end() { return rows.end(); }
    const_iterator begin() const { return rows.begin(); }
    const_iterator end() const { return rows.end(); }

private:
    bool success = true;
    std::vector<std::vector<DataType::Value>> rows;
    std::string error;
};