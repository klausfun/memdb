#include "result.h"

void Result::add_row(const Row& row) {
    rows.push_back(row);
}

bool Result::is_ok() const {
    return success;
}

std::string Result::get_error() {
    return error;
}