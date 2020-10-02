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

#include "SQLITE3.hpp"
#include "SQLITE3_QUERY.hpp"
#include <cassert>
#include <sqlite3.h>
int main () {
    SQLITE3 db("test.db"); // init database
    if (db.execute("CREATE TABLE test (id int PRIMARY KEY, data text);")) {
        abort();
    }

    // run complex query using SQLITE3_QUERY object
    SQLITE3_QUERY query = SQLITE3_QUERY("INSERT INTO test VALUES (?, ?),(?, ?);"); // ? will be replaced after bind
    query.add_binding("100", "foo", "200", "bar"); // add binding values
    if (db.execute(query)) { // execute query
        abort();
    }
    std::cout << "2 row inserted to table test" << std::endl;
    if (db.execute(query)) { // error because we execute the same insert query
        // good
    } else {
        abort();
    }

    // methods from SQLITE3_QUERY can be chained, however, no chained function call can be after add_binding
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test");
    if(!db.execute(query)){
        db.print_result();
    } else {
        abort();
    }

    // check result
    assert(db.get_result_row_count() == 2);
    assert(db.get_result_col_count() == 2);

    // check column name capture
    auto column_name = db.copy_column_names();
    assert(column_name->at(0) == "id");
    assert(column_name->at(1) == "data");

    // result is stored in a vector of SQLITE_ROW_VECTOR
    auto result = db.copy_result();
    assert(result->at(0).at(0) == "100");
    assert(result->at(0).at(1) == "foo");
    assert(result->at(1).at(0) == "200");
    assert(result->at(1).at(1) == "bar");

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
    result = db.copy_result();
    assert(result->at(0).at(0) == "Hello100");
    assert(result->at(1).at(0) == "Hello200");

    // drop table
    db.execute("DROP TABLE test;");
    std::cout << "Table test dropped" << std::endl;
    db.commit();

    return 0;
}