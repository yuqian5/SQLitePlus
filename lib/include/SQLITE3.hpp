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

#ifndef SQLITEPLUS_SQLITE3_HPP
#define SQLITEPLUS_SQLITE3_HPP

#include <sqlite3.h>
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
    explicit SQLITE3(const std::string &db_name = ""){
        // initialize db pointer
        db = std::make_shared<sqlite3*>();
        // open database if name is provided
        if (!db_name.empty()) {
            int rc = sqlite3_open(db_name.c_str(), db.get());
            if (rc != SQLITE_OK) { // check for error
                error_no = 1; // set error code

                sqlite3_close(*db);
                throw std::runtime_error("Unable to open database");
            }
            start_transaction();
        }

        // initialize result vector
        result = std::make_shared<std::vector<SQLITE_ROW_VECTOR>>();
        // initialize err_msg
        err_msg = std::make_shared<char*>();
    }

    /**
     * Non-construction-copyable
     */
    SQLITE3(const SQLITE3 &) = delete;

    /**
     * copyable
     */
    SQLITE3 &operator=(const SQLITE3 &rhs) {
        if (this == &rhs){ // self assignment guard
            return *this;
        }

        // copy shared_pointers
        this->db = rhs.db;
        this->err_msg = rhs.err_msg;
        this->err_msg_str = rhs.err_msg_str;
        this->result = rhs.result;

        return *this;
    }

    /**
     * Destructor
     */
    ~SQLITE3(){
        if (db) {
            sqlite3_close(*db);
        }
        if (err_msg) {
            sqlite3_free(err_msg.get());
        }
    }

    /**
     * Connect to db named db_name
     * @param db_name name of the database to open
     * @return 0 upon success, 1 upon failure
     */
    int open(std::string &db_name){
        if (db) { // can only bind to 1 database
            error_no = 2;
            return 1;
        } else {
            int rc = sqlite3_open(db_name.c_str(), db.get());

            if (rc != SQLITE_OK) { // check for error
                error_no = 1; // set error code

                sqlite3_close(*db);
                return 1;
            }

            start_transaction();
            return 0; // all good
        }
    }

    /**
     * Commit all change to database, then start a new transaction
     * @return 0 upon success, 1 upon failure
     */
    int commit(){
        int rc = sqlite3_exec(*db, "COMMIT;", nullptr, nullptr, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            error_no = 127;
            return 1;
        }
        start_transaction();
        return 0;
    }

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(SQLITE3_QUERY &query){
        // check if database connection is open
        if (!db) {
            error_no = 4;
            return 1;
        }

        // get query from SQLITE3_QUERY
        std::string prepared_query;
        try {
            prepared_query = query.bind().bound_query;
        } catch (std::out_of_range &e) {
            error_no = 3;
            return 1;
        }

        // clear result vector
        result->clear();

        // run query
        int rc = sqlite3_exec(*db, prepared_query.c_str(), &exec_callback, &result, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            error_no = 127;
            return 1;
        }
        return 0;
    }

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(std::string &query) {
        // check if database connection is open
        if (!db) {
            error_no = 4;
            return 1;
        }

        // clear result vector
        result->clear();

        // run query
        int rc = sqlite3_exec(*db, query.c_str(), &exec_callback, &result, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            error_no = 127;
            return 1;
        }
        return 0;
    }

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(const char* query) {
        // check if database connection is open
        if (!db) {
            error_no = 4;
            return 1;
        }

        // clear result vector
        result->clear();

        // run query
        int rc = sqlite3_exec(*db, query, &exec_callback, &result, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            error_no = 127;
            return 1;
        }
        return 0;
    }

    /**
     * Get the number of rows the
     * @return int
     */
    int get_result_row_count(){
        return result->size();
    }

    /**
     * Return the result of a query
     * @return shared pointer pointing to a copy of the result
     */
    std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> get_result_copy() {
        auto ret = std::make_shared<std::vector<SQLITE_ROW_VECTOR>>();
        std::copy(result->begin(), result->end(), std::back_inserter(*ret));
        return ret;
    }

    /**
     * @deprecated Deprecated due to possibility of unsafe memory access
     *
     * Return the result of a query
     * @return pointer to
     */
    const std::vector<SQLITE_ROW_VECTOR>* get_result() {
        return result.get();
    }

    /**
     * Print out result of statement
     */
    void print_result(){
        for (auto &x : *result){ // print results
            std::cout << "|";
            for (auto &y : x){
                std::cout << y << "|";
            }
            std::cout << "\n";
        }
    }

    /**
     * Get sqlite3 pointer, allowing user to expand the functions of SQLite
     * @return db
     */
    const sqlite3* getDB(){
        return *db;
    }

    /**
     * Read the class wide error_no and print parsed error to std::cerr
     */
    void perror() const{
        switch (error_no) {
            case 0:
                break;
            case 1:
                std::cerr << "SQLITE DATABASE OPEN FAILURE\n";
                break;
            case 2:
                std::cerr << "SQLITE DATABASE ALREADY OPENED, CREATE NEW OBJECT FOR NEW DATABASE\n";
                break;
            case 3:
                std::cerr << "Query Binding Failed\n";
                break;
            case 4:
                std::cerr << "No database connected\n";
                break;
            case 126:
                std::cerr << err_msg_str << std::endl;
                break;
            case 127:
                std::cerr << *err_msg << std::endl;
                break;
        }
    }

    /**
     * Add simple user defined function to database
     * @param name name of function
     * @param argc number of argument a function take
     * @param lambda function implementation
     * \lambda_arg
     *      (sqlite3_context* context, int argc, sqlite3_value** value)
     * @return 0 upon success, 1 upon failure
     */
    int add_function(const std::string& name,int argc, void (*lambda)(sqlite3_context*, int, sqlite3_value**)){
        // add function to database
        int rc = sqlite3_create_function(*db, name.c_str(),
                                         argc, SQLITE_UTF8,
                                         nullptr,
                                         lambda,
                                         nullptr,
                                         nullptr);

        // check success
        if (rc != SQLITE_OK) {
            error_no = 126;
            err_msg_str = sqlite3_errmsg(*db);
            return 1;
        }

        return 0;
    }

private:
    /**
     * Begin a new transaction
     * @return 0 upon success, 1 upon failure
     */
    int start_transaction(){
        int rc = sqlite3_exec(*db, "BEGIN;", nullptr, nullptr, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            error_no = 127;
            return 1;
        }
        return 0;
    }

    /**
     * Collect results and put into result vector
     * @param ptr pointer to result vector
     * @param argc number of column
     * @param argv result char**
     * @param col_name column name
     * @return 0
     */
    static int exec_callback(void *ptr, int argc, char *argv[], char *col_name[]){
        auto *result = reinterpret_cast<std::unique_ptr<std::vector<SQLITE_ROW_VECTOR>> *>(ptr);

        // get result
        SQLITE_ROW_VECTOR row;
        for(int i = 0; i < argc; i++) {
            row.push_back(std::string(argv[i] ? argv[i] : "NULL"));
        }

        // push SQLITE_ROW_VECTOR to result vector
        result->get()->push_back(row);

        return 0;
    }

public:
    char error_no; // class wide error code

private:
    // sqlite objects
    std::shared_ptr<sqlite3*> db;
    std::shared_ptr<char*> err_msg;
    std::string err_msg_str;
    std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> result;
};


#endif //SQLITEPLUS_SQLITE3_HPP
