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
#include <mutex>
#include <utility>

#include "SQLITE3_QUERY.hpp"

/**
 * \private
 */
enum {NO_ERROR, OPEN_ERROR, OVERRIDE_ERROR, QUERY_BINDING_ERROR, UNINITIALIZED_ERROR, EXECUTION_ERROR};

/**
 * \private
 */
typedef std::vector<std::string> SQLITE_ROW_VECTOR;

/**
 * \private
 */
struct Callback_Data {
    Callback_Data(std::shared_ptr<SQLITE_ROW_VECTOR> col, std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> rows) {
        this->col = std::move(col);
        this->rows = std::move(rows);
    }

    std::shared_ptr<SQLITE_ROW_VECTOR> col;
    std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> rows;
};

/**
 * Wrapper Library for sqlite3
 */
class SQLITE3 {
public:
    /**
     * Constructor
     * @param db_name name of database to open
     */
    explicit SQLITE3(const std::string &db_name = "") {
        // initialize db pointer
        db = std::make_shared<sqlite3 *>();
        // open database if name is provided
        if (!db_name.empty()) {
            int rc = sqlite3_open(db_name.c_str(), db.get());
            if (rc != SQLITE_OK) { // check for error
                error_no = OPEN_ERROR; // set error code

                sqlite3_close(*db);
                throw std::runtime_error("Unable to open database");
            }
            start_transaction();
        }

        // initialize result and column vector
        column_name = std::make_shared<SQLITE_ROW_VECTOR>();
        result = std::make_shared<std::vector<SQLITE_ROW_VECTOR>>();
        // initialize err_msg
        err_msg = std::make_shared<char *>();

        // initialize mutex
        exec_lock = std::make_shared<std::mutex>();
    }

    /**
     * copy construction
     */
    SQLITE3(const SQLITE3 &rhs) {
        // copy shared_pointers
        this->db = rhs.db;
        this->err_msg = rhs.err_msg;
        this->err_msg_str = rhs.err_msg_str;
        this->result = rhs.result;
        this->column_name = rhs.column_name;
        this->exec_lock = rhs.exec_lock;
        this->error_no = rhs.error_no;
    }

    /**
     * copy assign
     */
    SQLITE3 &operator=(const SQLITE3 &rhs) {
        if (this == &rhs) { // self assignment guard
            return *this;
        }

        // close previous database if connection exist
        if (*db) {
            sqlite3_close(*db);
        }
        if (*err_msg) {
            sqlite3_free(*err_msg);
        }

        // copy shared_pointers
        this->db = rhs.db;
        this->err_msg = rhs.err_msg;
        this->err_msg_str = rhs.err_msg_str;
        this->result = rhs.result;
        this->column_name = rhs.column_name;
        this->exec_lock = rhs.exec_lock;
        this->error_no = rhs.error_no;

        return *this;
    }

    /**
     * Destructor
     */
    ~SQLITE3() {
        if (*db) {
            sqlite3_close(*db);
        }
        if (*err_msg) {
            sqlite3_free(*err_msg);
        }
    }

    /**
     * Connect to db named db_name
     * @param db_name name of the database to open
     * @return 0 upon success, 1 upon failure
     */
    int open(std::string &db_name) {
        // close previous connection if needed
        if (*db) {
            sqlite3_close(*db);
        }
        if (*err_msg) {
            sqlite3_free(*err_msg);
        }

        // open connection
        int rc = sqlite3_open(db_name.c_str(), db.get());

        if (rc != SQLITE_OK) { // check for error
            error_no = OPEN_ERROR; // set error code

            sqlite3_close(*db);
            return 1;
        }

        start_transaction();
        return 0; // all good
    }

    /**
     * Commit all change to database, then start a new transaction
     * @return 0 upon success, 1 upon failure
     */
    int commit() {
        int rc = sqlite3_exec(*db, "COMMIT;", nullptr, nullptr, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            // copy error message and free memory
            err_msg_str = std::string(*err_msg);
            sqlite3_free(*err_msg);
            error_no = EXECUTION_ERROR;

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
    int execute(SQLITE3_QUERY &query) {
        exec_lock->lock(); // lock exec

        // check if database connection is open
        if (!*db) {
            error_no = UNINITIALIZED_ERROR;
            exec_lock->unlock(); // unlock exec

            return 1;
        }

        // get query from SQLITE3_QUERY
        std::string prepared_query;
        try {
            prepared_query = query.bind().bound_query;
        } catch (std::out_of_range &e) {
            error_no = QUERY_BINDING_ERROR;
            exec_lock->unlock(); // unlock exec

            return 1;
        }

        // clear result and column vector
        result->clear();
        column_name->clear();

        // run query
        auto data_pack = Callback_Data(column_name, result);
        int rc = sqlite3_exec(*db, prepared_query.c_str(), &exec_callback, &data_pack, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            // copy error message or get error message
            if (*err_msg) {
                err_msg_str = std::string(*err_msg);
                sqlite3_free(*err_msg);
            } else {
                err_msg_str = std::string(sqlite3_errmsg(*db));
            }

            error_no = EXECUTION_ERROR;

            exec_lock->unlock(); // unlock exec

            return 1;
        }

        exec_lock->unlock(); // unlock exec
        return 0;
    }

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(std::string &query) {
        exec_lock->lock(); // lock exec

        // check if database connection is open
        if (!*db) {
            error_no = UNINITIALIZED_ERROR;
            exec_lock->unlock(); // unlock exec

            return 1;
        }

        // clear result and column vector
        result->clear();
        column_name->clear();

        // run query
        auto data_pack = Callback_Data(column_name, result);
        int rc = sqlite3_exec(*db, query.c_str(), &exec_callback, &data_pack, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            // copy error message or get error message
            if (*err_msg) {
                err_msg_str = std::string(*err_msg);
                sqlite3_free(*err_msg);
            } else {
                err_msg_str = std::string(sqlite3_errmsg(*db));
            }

            error_no = EXECUTION_ERROR;

            exec_lock->unlock(); // unlock exec

            return 1;
        }

        exec_lock->unlock(); // unlock exec
        return 0;
    }

    /**
     * Execute query
     * @param query
     * @return 0 upon success, 1 upon failure
     */
    int execute(const char *query) {
        exec_lock->lock(); // lock exec

        // check if database connection is open
        if (!*db) {
            error_no = UNINITIALIZED_ERROR;
            exec_lock->unlock(); // unlock exec

            return 1;
        }

        // clear result and column vector
        result->clear();
        column_name->clear();

        // run query
        auto data_pack = Callback_Data(column_name, result);
        int rc = sqlite3_exec(*db, query, &exec_callback, &data_pack, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            // copy error message or get error message
            if (*err_msg) {
                err_msg_str = std::string(*err_msg);
                sqlite3_free(*err_msg);
            } else {
                err_msg_str = std::string(sqlite3_errmsg(*db));
            }

            error_no = EXECUTION_ERROR;

            exec_lock->unlock(); // unlock exec

            return 1;
        }

        exec_lock->unlock(); // unlock exec
        return 0;
    }

    /**
     * Return the a copy of the column names for the result of the last query
     * @return shared pointer pointing to a copy of the column name
     */
    std::shared_ptr<SQLITE_ROW_VECTOR> copy_column_names() const {
        exec_lock->lock(); // lock exec

        // make copy of result
        auto ret = std::make_shared<SQLITE_ROW_VECTOR>();
        std::copy(column_name->begin(), column_name->end(), std::back_inserter(*ret));

        exec_lock->unlock(); // unlock exec

        return ret;
    }

    /**
     * Get the number of col returned
     * @return number of col
     */
    int get_result_col_count() const {
        return result->at(0).size();
    }

    /**
     * Get the number of row returned
     * @return number of row
     */
    int get_result_row_count() const {
        return result->size();
    }

    /**
     * Return the a copy of the result of the last query
     * @return shared pointer pointing to a copy of the result
     */
    std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> copy_result() const {
        exec_lock->lock(); // lock exec

        // make copy of result
        auto ret = std::make_shared<std::vector<SQLITE_ROW_VECTOR>>();
        std::copy(result->begin(), result->end(), std::back_inserter(*ret));

        exec_lock->unlock(); // unlock exec

        return ret;
    }

    /**
     * @deprecated Deprecated due to possibility of unsafe memory access
     *
     * Return the result of a query
     * @return pointer to
     */
    [[deprecated]]
    const std::vector<SQLITE_ROW_VECTOR> *get_result() const {
        return result.get();
    }

    /**
     * Print out result of a query
     */
    void print_result() const {
        // copy result and column names
        auto column_name_copy = copy_column_names();
        auto result_copy = copy_result();

        // print column names
        std::cout << "|";
        for (auto &name : *column_name_copy) {
            std::cout << name << "|";
        }
        std::cout << std::endl;

        // print rows
        for (SQLITE_ROW_VECTOR &row : *result_copy) {
            std::cout << "|";
            for (auto &col : row) {
                std::cout << col << "|";
            }
            std::cout << "\n";
        }
    }

    /**
     * Get sqlite3 pointer, allowing user to expand the functions of SQLite
     * @return pointer to db
     */
    const sqlite3 *get_db() const {
        return *db;
    }

    /**
     * Read the class wide error_no and print parsed error to std::cerr
     */
    void perror() {
        switch (error_no) {
            case NO_ERROR:
                break;
            case OPEN_ERROR:
                std::cerr << "SQLITE DATABASE OPEN FAILURE\n";
                break;
            case OVERRIDE_ERROR:
                std::cerr << "SQLITE DATABASE ALREADY OPENED, CREATE NEW OBJECT FOR NEW DATABASE\n";
                break;
            case QUERY_BINDING_ERROR:
                std::cerr << "Query Binding Failed\n";
                break;
            case UNINITIALIZED_ERROR:
                std::cerr << "No database connected\n";
                break;
            case EXECUTION_ERROR:
                std::cerr << err_msg_str << std::endl;
                break;
        }

        error_no = NO_ERROR;
    }

    /**
     * Add simple user defined function to database
     * @param name name of function
     * @param argc number of argument a function take
     * @param lambda function implementation
     * @lambda_arg void (sqlite3_context* context, int argc, sqlite3_value** value)
     * @return 0 upon success, 1 upon failure
     */
    int add_function(const std::string &name, int argc, void (*lambda)(sqlite3_context *, int, sqlite3_value **)) {
        // add function to database
        int rc = sqlite3_create_function(*db,
                                         name.c_str(),
                                         argc, SQLITE_UTF8,
                                         nullptr,
                                         lambda,
                                         nullptr,
                                         nullptr);

        // check success
        if (rc != SQLITE_OK) {
            error_no = EXECUTION_ERROR;
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
    int start_transaction() {
        int rc = sqlite3_exec(*db, "BEGIN;", nullptr, nullptr, err_msg.get());
        if (rc != SQLITE_OK) { // check for error
            // copy error message and free memory
            err_msg_str = std::string(*err_msg);
            sqlite3_free(*err_msg);
            error_no = EXECUTION_ERROR;

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
    static int exec_callback(void *ptr, int argc, char *argv[], char *col_name[]) {
        auto *data = reinterpret_cast<Callback_Data *>(ptr);

        // record column name if needed
        if (data->col->empty()) {
            for (int i = 0; i < argc; ++i) {
                data->col->push_back(std::string(col_name[i] ? col_name[i] : "NULL"));
            }
        }

        // get result
        SQLITE_ROW_VECTOR row;
        for (int i = 0; i < argc; i++) {
            row.push_back(std::string(argv[i] ? argv[i] : "NULL"));
        }

        // push SQLITE_ROW_VECTOR to result vector
        data->rows->push_back(row);

        return 0;
    }

public:
    char error_no{}; // class wide error code

private:
    // sqlite objects
    std::shared_ptr<sqlite3 *> db;
    std::shared_ptr<char *> err_msg;
    std::string err_msg_str;

    // query results
    std::shared_ptr<SQLITE_ROW_VECTOR> column_name; // vector storing result column name
    std::shared_ptr<std::vector<SQLITE_ROW_VECTOR>> result; // result stored in matrix format

    // To prevent concurrent access
    std::shared_ptr<std::mutex> exec_lock;
};


#endif //SQLITEPLUS_SQLITE3_HPP
