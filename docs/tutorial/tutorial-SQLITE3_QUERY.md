# Tutorial SQLITE3_QUERY
A basic tutorial on SQLITE3_QUERY

### Create a query
``` c++
    SQLITE3_QUERY query;
```
    
or you can create a query with a sql template
    
``` c++
    SQLITE#_QUERY query("SELECT * FROM ? WHERE id > ?;");
```
    
### Add SQL query template to a query
``` c++
    query.set_query_template("SELECT * FROM ? WHERE id > ?;");
```
    
### Add binding to a query
``` c++
    query.add_binding("TestTable");
```
    
or

``` c++
    std::string table_name = "TestTable";
    query.add_binding(table_name);
```

or 

``` c++
    char* table_name = "TestTable";
    query.add_binding(table_name);
```
    
or you can add multiple bindings at the same time

``` c++
    query.add_binding("TestTable", "100"); 
    // you can use const char*, char* or std::string
```
    
### Delete all binding
``` c++
    query.reset_binding();
```
    
### Create a SQL query
``` c++
    query.bind()
```
    
### Get the created SQL query
``` c++
    std::string sql_query = query.bound_query;
```
    
### Function calls can be chained together
``` c++
    // good
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test");
    
    // error
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test").bind();
```
    
You cannot chain function calls after add_binding.