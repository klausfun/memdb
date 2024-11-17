#include "result.h"

// Метод для добавления строки в результат
void Result::add_row(const Row& row) {
    rows.push_back(row);
}

// Метод для проверки успешности запроса
bool Result::is_ok() const {
    return success;
}

std::string Result::get_error() {
    return error;
}
