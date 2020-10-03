#include "SQLITE3_QUERY.hpp"

#include <cassert>

int main () {
    SQLITE3_QUERY query;

    // check initialization
    assert(query.query_template.empty());

    // check
    query.set_query_template("? ? ?");

    // check add_binding and bind
    char* s1 =  "abc";
    query.add_binding(s1);
    query.add_binding("def");
    query.add_binding(std::string("ghi"));
    assert(query.bind().bound_query == "'abc' 'def' 'ghi'");

    // check copy construct
    SQLITE3_QUERY query2 = query;
    assert(query.bind().bound_query == query2.bind().bound_query);

    // check copy assignment
    SQLITE3_QUERY query3 = query;
    assert(query.bind().bound_query == query3.bind().bound_query);

    // check reset binding, and pack binding
    query3.reset_binding();
    query3.add_binding("abc", "def", "ghi");
    assert(query.bind().bound_query == query3.bind().bound_query);
}