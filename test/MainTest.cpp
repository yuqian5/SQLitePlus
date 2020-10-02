#include "SQLITE3.hpp"
#include "SQLITE3_QUERY.hpp"

#include <cassert>
#include <sqlite3.h>

int main () {
    SQLITE3 db("test.db"); // init database
    if (db.execute("CREATE TABLE test (id int PRIMARY KEY, data text);")) {
        abort();
    }

    return 0;
}
