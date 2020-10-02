//demo

#include <iostream>
#include <sqlite3.h>

#include "SQLITE3_QUERY.hpp"
#include "SQLITE3.hpp"

int main() {
    std::cout << "test.db created" << std::endl;
    SQLITE3 db("test.db"); // init database


    return 0;
}
