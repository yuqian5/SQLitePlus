# Tutorial SQLITE3
A basic tutorial on SQLITE3

### Open database
``` c++
    SQLITE3 db("test.db");
```
    
or
``` c++
    SQLITE3 db;
    db.open("test.db");
```
    
Note: Once SQLITE3 established a connection to a database, 
it cannot open another connection to another database. You will 
need to create another SQLITE3.
    
### Run a query
``` c++
    int rc = db.execute("CREATE TABLE test (id int PRIMARY KEY, data text);")
    if (rc) { // return 1 when failed
        // handle error
    } else {
        // we are good
    }
```
    
### Commit a query
``` c++
    db.commit();
```
    
### Get the result of the last query executed
``` c++
    auto result = db.copy_result(); 
    // returns a shared pointer that points to a vector of SQLITE_ROW_VECTOR
```
    
### Get the column names of the result of the last query executed
``` c++
    auto columns = db.copy_column_names(); 
    // returns a shared pointer that points to a vector of strings
```
    
### Get number of rows returned 
``` c++
    int row_count = db.get_result_row_count();
```
    
### Get number of columns returned
``` c++
    int column_count = db.get_result_col_count();
```
    
### Print the result of the last query executed
``` c++
    db.print_result();
```
    
### Get the raw sqlite pointer
``` c++
    db.get_db();
```

### Get the error message from the last executed query, if exist
``` c++
    db.perror();
```

### Add a user defined function to database
``` c++
    db.add_function("function_name", 1, [](sqlite3_context* c, int argc, sqlite3_value** value){ //implementation });
```

### Demo Program   
``` c++
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
```