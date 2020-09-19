//demo

#include <iostream>

#include "SQLITE3_QUERY.hpp"
#include "SQLITE3.hpp"

int main() {
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
    if (db.execute(query)) { // error
        db.perror();
    }
    std::cout << "Repeated insertion failed" << std::endl;

    // methods can be chained
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test");
    if(!db.execute(query)){
        db.print_result();
    } else {
        db.perror();
    }

    // result is stored in a vector of SQLITE_ROW_VECTOR
    auto result = db.get_result();
    std::cout << "The first element of first row is: " << result->at(0).at(0) << std::endl;

    // commit to save changes
    db.commit();
    std::cout << "Changes committed" << std::endl;

    // drop table
    db.execute("DROP TABLE test;");
    std::cout << "Table test dropped" << std::endl;
    db.commit();

    return 0;
}
