#include <iostream>
#include "../include/SQLITE3_QUERY.hpp"
#include "../include/SQLITE3.hpp"

int main() {
    SQLITE3_QUERY query = SQLITE3_QUERY("SELECT * FROM node AND WHERE id > ?;");
    query.add_binding("3267939850", "3267940116");
    SQLITE3 db("edmonton.db");
    if (!db.execute(query)) {
        for (auto &x : *db.get_result()){
            std::cout << "|";
            for (auto &y : x){
                std::cout << y << "|";
            }
            std::cout << "\n";
        }
    } else {
        db.perror();
    }


    return 0;
}
