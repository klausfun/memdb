#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include "src/db.hpp"

int main() {
    std::string input_table = "  create   table   users_admin$key   (  { key ,  autoincrement  }    id   :   int32 ,   {  unique }   login  :   string[32] ,   password_hash  :   bytes[8]  ,  is_admin  :   bool   =   true  )";
    std::string input_insert = "insert (\n"
                         "is_admin = true,\n"
                         "login = \"admin\",\n"
                         "password_hash = 0x0000000000000000\n"
                         ") to users";
    std::string input_select = "select id, login from users where is_admin || id < 10";
    std::string input_update = "update users set login = login + \"_deleted\", is_admin = false where\n"
                         "password_hash < 0x00000000ffffffff";
    std::string input_delete = "delete users where |login| % 2 = 0";
    std::string input_index = "create ordered index on users by login";

    Tokenizer tokenizer;
    std::vector<Token> tokens = tokenizer.tokenize(input_table);
    tokenizer.printTokens(tokens);

    Database db;
    db.execute(input_table);
    db.execute(input_insert);
    db.execute(input_select);
    db.execute(input_update);
    db.execute(input_delete);
    db.execute(input_index);

    return 0;
}
