//demo
//
// Created by Kerry Cao on 2020-09-18.
// SQLitePlus
//    Copyright (C) <2020>  <Yuqian Cao> (kcyq98@gmail.com)
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA
//

#include <iostream>
#include <sqlite3.h>

#include "SQLITE3_QUERY.hpp"
#include "SQLITE3.hpp"

int main() {
    std::cout << "test.db created" << std::endl;
    SQLITE3 db("test.db"); // init database


    // run execute plain string query
    if (db.execute("CREATE TABLE test (id int PRIMARY KEY, data text);")) {
        db.perror(); // in case of error, perror will print detail to stderr
    }
    std::cout << "Table test created" << std::endl;


    // run complex query using SQLITE3_QUERY object
    SQLITE3_QUERY query = SQLITE3_QUERY("INSERT INTO test VALUES (?, ?),(?, ?);"); // ? will be replaced after bind
    query.add_binding("100", "foo", "200", "bar"); // add binding values
    if (db.execute(query)) { // execute query
        db.perror();
    }
    std::cout << "2 row inserted to table test" << std::endl;
    if (db.execute(query)) { // error because we execute the same insert query
        db.perror(); // get error message
        std::cerr.flush();
    }
    std::cout << "Repeated insertion failed" << std::endl;


    // methods from SQLITE3_QUERY can be chained, however, no chained function call can be after add_binding
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test");
    if(!db.execute(query)){
        db.print_result();
    } else {
        db.perror();
    }


    // get number of rows returned
    std::cout << db.get_result_row_count() << " rows returned" << std::endl;


    // get number of cols returned
    std::cout << db.get_result_col_count() << " cols returned" << std::endl;


    // get column names
    auto column_name = db.copy_column_names();
    std::cout << "|";
    for (auto &name : *column_name) {
        std::cout << name << "|";
    }
    std::cout << std::endl;


    // result is stored in a vector of SQLITE_ROW_VECTOR
    auto result = db.copy_result();
    std::cout << "The first element of first row is: " << result->at(0).at(0) << std::endl;


    // commit to save changes
    db.commit();
    std::cout << "Changes committed" << std::endl;


    // add user defined function to database
    db.add_function("PrintHello", //name of function
                    1, // number of argument the UDF take
                    [](sqlite3_context* c, int argc, sqlite3_value** value){ // function implementation
                        int id = sqlite3_value_int(value[0]);
                        std::string result = std::string("Hello" + std::to_string(id));

                        sqlite3_result_text(c, result.c_str(), result.length(), nullptr);
                    });
    db.execute("SELECT PrintHello(id) FROM TEST;");
    db.print_result();


    // drop table
    db.execute("DROP TABLE test;");
    std::cout << "Table test dropped" << std::endl;
    db.commit();

    return 0;
}