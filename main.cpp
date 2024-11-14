#include <iostream>
#include "src/db.h"
#include <regex>

std::string sanitize_query(const std::string& query) {
    std::string sanitized = query;
    std::regex ws_re("\\s+");
    sanitized = std::regex_replace(sanitized, ws_re, " ");

    return sanitized;
}

int main() {
    Database db;
//    std::string query = "Create table users ({key, autoincrement} id :\n"
//                        "int32, {unique} login: string[32], password_hash: bytes[8], is_admin:\n"
//                        "bool = false)";
//    auto result = db.execute(query);
    std::string query = "  create   table   users   (  { key ,  autoincrement  }    id   :   int32 ,   {  unique }   login  :   string[32] ,   password_hash  :   bytes[8]  ,  is_admin  :   bool   =   false  )";
    query = sanitize_query(query);
    std::cout << query << std::endl;

    return 0;
}
