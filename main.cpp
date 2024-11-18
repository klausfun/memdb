#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <fstream>
#include "src/db.hpp"

void printFileHex(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    // Читаем файл побайтово и выводим в hex формате
    unsigned char byte;
    int count = 0;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        printf("%02X ", byte);
        if (++count % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

int main() {
//    std::string input_table = "  create   table     users   ( { key  , autoincrement  }    id   :   int32,   {  unique }   login  :   string[32],    password_hash  :   bytes[8]  ,  is_admin  :   bool   =   true  )";
//    std::string input_insert1 = "insert (\n"
//                         "is_admin = false,\n"
//                         "login = \"w ?!\",\n"
//                         "password_hash = 0x000200340056000\n"
//                         ") to users";
    std::string input_insert2 = "insert (\n "
                                "\"dir\",\n"
                                "0x5522000033300000\n) to users";
//    std::string input_select = "select id, login from users where is_admin || id < 10";
//    std::string input_update = "update users set login = login + \"_deleted\", is_admin = false where\n"
//                         "password_hash < 0x00000000ffffffff";
//    std::string input_delete = "delete users where |login| % 2 = 0";
//    std::string input_index = "create ordered index on users by login";

//    Tokenizer tokenizer;
//    std::vector<Token> tokens = tokenizer.tokenize(input_table);
//    tokenizer.printTokens(tokens);

    Database db;
    db.load_from_json("database2.json");

//    db.execute(input_table);
//    db.execute(input_insert1);
    db.execute(input_insert2);
//    db.execute(input_select);
//    db.execute(input_update);
//    db.execute(input_delete);
//    db.execute(input_index);

    db.save_to_json("database2.json");
//    {
//        std::ofstream output_file("database.bin", std::ios::binary);
//        if (!output_file) {
//            throw std::runtime_error("Cannot open file for writing");
//        }
//        db.save_to_file(output_file);
//    }
//    {
//        std::ifstream input_file("database.bin", std::ios::binary);
//        if (!input_file) {
//            throw std::runtime_error("Cannot open file for reading");
//        }
//        db.load_from_file(input_file);
//    }
//    {
//        std::ofstream output_file("database2.bin", std::ios::binary);
//        if (!output_file) {
//            throw std::runtime_error("Cannot open file for writing");
//        }
//        db.save_to_file(output_file);
//    }
//
//    std::cout << "File contents in hex:" << std::endl;
//    printFileHex("database.bin");
//
//    std::cout << "\n\nFile contents in hex:" << std::endl;
//    printFileHex("database2.bin");

    return 0;
}
