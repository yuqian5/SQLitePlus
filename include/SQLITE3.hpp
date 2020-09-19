//
// Created by Kerry Cao on 2020-09-18.
//

#ifndef SQLITEPLUS_SQLITE3_HPP
#define SQLITEPLUS_SQLITE3_HPP

#include <sqlite3.h>
#include <boost/format.hpp>
#include <iostream>
#include <memory>
#include <functional>

#include "SQLITE3_QUERY.hpp"

typedef std::vector<std::string> SQLITE_ROW_VECTOR;

class SQLITE3 {
public:
    /**
     * Constructor
     * @param db_name name of database to open
     */
    explicit SQLITE3(const std::string &db_name = "");

    /**
     * Non-construction-copyable
     */
    SQLITE3(const SQLITE3 &) = delete;

    /**
     * Non-copyable
     * @return SQLITE3
     */
    SQLITE3 &operator=(const SQLITE3 &) = delete;

    /**
     * Destructor
     */
    ~SQLITE3();

    /**
     * Connect to db named db_name
     * @param db_name name of the database to open
     * @return 0 upon success, 1 upon failure
     */
    int open(std::string &db_name);

    /**
     * Commit all change to database, then start a new transaction
     * @return 0 upon success, 1 upon failure
     */
    int commit();

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(SQLITE3_QUERY &query);

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(std::string &query);

    /**
     * Get the number of rows the
     * @return int
     */
    int get_result_row_count(){
        return result->size();
    }

    /**
     * Return the result of a query
     * @return std::vector<std::string>*
     */
    std::vector<SQLITE_ROW_VECTOR>* get_result();

    /**
     * Read the class wide error_no and print parsed error to std::cerr
     */
    void perror() const;

private:
    /**
     * Begin a new transaction
     * @return 0 upon success, 1 upon failure
     */
    int start_transaction();

    /**
     * Collect results and put into result vector
     * @param ptr pointer to result vector
     * @param argc number of column
     * @param argv result char**
     * @param col_name column name
     * @return 0
     */
    static int exec_callback(void *ptr, int argc, char *argv[], char *col_name[]);

public:
    char error_no; // class wide error code

private:
    // sqlite objects
    sqlite3 *db;
    char *err_msg;
    sqlite3_stmt *stmt;
    std::unique_ptr<std::vector<SQLITE_ROW_VECTOR>> result;
};


#endif //SQLITEPLUS_SQLITE3_HPP
