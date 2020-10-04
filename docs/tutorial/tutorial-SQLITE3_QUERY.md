# Tutorial SQLITE3_QUERY
A basic tutorial on SQLITE3_QUERY

### Create a query
    SQLITE3_QUERY query;
    
or you can create a query with a sql template
    
    SQLITE#_QUERY query("SELECT * FROM ? WHERE id > ?;");
    
### Add SQL query template to a query
    query.set_query_template("SELECT * FROM ? WHERE id > ?;");
    
### Add binding to a query
    query.add_binding("TestTable");
    
or

    std::string table_name = "TestTable";
    query.add_binding(table_name);
    
or 

    char* table_name = "TestTable";
    query.add_binding(table_name);
    
or you can add multiple bindings at the same time

    query.add_binding("TestTable", "100"); 
    // you can use const char*, char* or std::string
    
### Delete all binding
    query.reset_binding();
    
### Create a SQL query
    query.bind()
    
### Get the created SQL query
    std::string sql_query = query.bound_query;
    
### Function calls can be chained together
    // good
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test");
    
    // error
    query.set_query_template("SELECT * FROM ?;").reset_binding().add_binding("test").bind();
    
You cannot chain function calls after add_binding.